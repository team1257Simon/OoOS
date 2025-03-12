/*
 * Implement fast CRC32C with PCLMULQDQ instructions. (x86_64)
 *
 * The white papers on CRC32C calculations with PCLMULQDQ instruction can be
 * downloaded from:
 * http://www.intel.com/content/dam/www/public/us/en/documents/white-papers/crc-iscsi-polynomial-crc32-instruction-paper.pdf
 * http://www.intel.com/content/dam/www/public/us/en/documents/white-papers/fast-crc-computation-paper.pdf
 *
 * Copyright (C) 2012 Intel Corporation.
 * Copyright 2024 Google LLC
 *
 * Authors:
 *	Wajdi Feghali <wajdi.k.feghali@intel.com>
 *	James Guilford <james.guilford@intel.com>
 *	David Cote <david.m.cote@intel.com>
 *	Tim Chen <tim.c.chen@linux.intel.com>
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * OpenIB.org BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
# ISCSI CRC 32 Implementation with crc32 and pclmulqdq Instruction
# Define threshold below which buffers are considered "small" and routed to
# regular CRC code that does not interleave the CRC instructions.
# uint32_t crc32c_x86_3way(uint32_t crc, const uint8_t *buffer, size_t len);
.text
.global crc32c_x86_3way
.type   crc32c_x86_3way, @function
crc32c_x86_3way:
	cmp	$200, %rdx
	jb .Lsmall
	################################################################
	## 1) ALIGN:
	################################################################
	mov	%esi, %ecx
	neg	%ecx
	and	$7, %ecx	# calculate the misalignment amount of the address
	je .Laligned	# Skip if aligned
	# Process 1 <= %ecx <= 7 bytes individually in order to align
	# the remaining data to an 8-byte boundary.
.Ldo_align:
	movq (%rsi), %rax
	add	%rcx, %rsi
	sub	%rcx, %rdx
.Lalign_loop:
	crc32b	%al, %edi		# compute crc32 of 1-byte
	shr	$8, %rax		    # get next byte
	dec	%ecx
	jne .Lalign_loop
.Laligned:
	################################################################
	## 2) PROCESS BLOCK:
	################################################################
	cmp	$128*24, %rdx
	jae .Lfull_block
.Lpartial_block:
	# Compute floor(%rdx / 24) to get num qwords to process from each lane.
	imul $2731, %edx, %eax	# 2731 = ceil(2^16 / 24)
	shr	$16, %eax
	jmp	.Lcrc_3lanes
.Lfull_block:
	# Processing 128 qwords from each lane.
	mov	$128, %eax
	################################################################
	## 3) CRC each of three lanes:
	################################################################
.Lcrc_3lanes:
	xor	%r8,%r8
	xor %r9,%r9
	mov	%eax, %ecx
	shl	$3, %ecx		# num bytes to process from each lane
	sub	$5, %eax		# 4 for 4x_loop, 1 for special last iter
	jl .Lcrc_3lanes_4x_done
	# Unroll the loop by a factor of 4 to reduce the overhead of the loop bookkeeping instructions, which can compete with crc32q for the ALUs.
.Lcrc_3lanes_4x_loop:
	crc32q (%rsi), %rdi
	crc32q (%rsi,%rcx), %r8
	crc32q (%rsi,%rcx,2), %r9
	crc32q 8(%rsi), %rdi
	crc32q 8(%rsi,%rcx), %r8
	crc32q 8(%rsi,%rcx,2), %r9
	crc32q 16(%rsi), %rdi
	crc32q 16(%rsi,%rcx), %r8
	crc32q 16(%rsi,%rcx,2), %r9
	crc32q 24(%rsi), %rdi
	crc32q 24(%rsi,%rcx), %r8
	crc32q 24(%rsi,%rcx,2), %r9
	add	$32, %rsi
	sub	$4, %eax
	jge	.Lcrc_3lanes_4x_loop
.Lcrc_3lanes_4x_done:
	add	$4, %eax
	jz	.Lcrc_3lanes_last_qword
.Lcrc_3lanes_1x_loop:
	crc32q (%rsi), %rdi
	crc32q (%rsi,%rcx), %r8
	crc32q (%rsi,%rcx,2), %r9
	add	$8, %rsi
	dec	%eax
	jnz	.Lcrc_3lanes_1x_loop
.Lcrc_3lanes_last_qword:
	crc32q	(%rsi), %rdi
	crc32q	(%rsi,%rcx), %r8
	################################################################
	## 4) Combine three results:
	################################################################
	lea	(K_table-8)(%rip), %rax		# first entry is for idx 1
	pmovzxdq (%rax,%rcx), %xmm0	    # 2 consts: K1:K2
	lea	(%ecx,%ecx,2), %eax         # %ecx * 3
	sub	%rax, %rdx			        # %rdx -= %ecx * 3
	movq %rdi, %xmm1			    # CRC for block 1
	pclmulqdq $0x00, %xmm0, %xmm1	# Multiply by K2
	movq %r8, %xmm2			        # CRC for block 2
	pclmulqdq $0x10, %xmm0, %xmm2	# Multiply by K1
	pxor %xmm2,%xmm1
	movq %xmm1, %rax
	xor	(%rsi,%rcx,2), %rax
	mov	%r9, %rdi
	crc32 %rax, %rdi
	lea	8(%rsi,%rcx,2), %rsi
	################################################################
	## 5) If more blocks remain, goto (2):
	################################################################
	cmp	$128*24, %rdx
	jae	.Lfull_block
	cmp	$200, %rdx
	jae	.Lpartial_block
	#######################################################################
	## 6) Process any remainder without interleaving:
	#######################################################################
.Lsmall:
	test %edx, %edx
	jz .Ldone
	mov	%edx, %eax
	shr	$3, %eax
	jz .Ldo_dword
.Ldo_qwords:
	crc32q (%rsi), %rdi
	add	$8, %rsi
	dec	%eax
	jnz	.Ldo_qwords
.Ldo_dword:
	test $4, %edx
	jz .Ldo_word
	crc32l (%rsi), %edi
	add	$4, %rsi
.Ldo_word:
	test $2, %edx
	jz .Ldo_byte
	crc32w (%rsi), %edi
	add	$2, %rsi
.Ldo_byte:
	test $1, %edx
	jz .Ldone
	crc32b (%rsi), %edi
.Ldone:
	mov	%edi, %eax
    ret
.size crc32c_x86_3way, .-crc32c_x86_3way
.section .rodata, "a", @progbits
	################################################################
	## PCLMULQDQ tables
	## Table is 128 entries x 2 words (8 bytes) each
	################################################################
.align 8
.type K_table, @object
K_table:
	.long 0x493C7D27, 0x00000001
	.long 0xBA4FC28E, 0x493C7D27
	.long 0xDDC0152B, 0xF20C0DFE
	.long 0x9E4ADDF8, 0xBA4FC28E
	.long 0x39D3B296, 0x3DA6D0CB
	.long 0x0715CE53, 0xDDC0152B
	.long 0x47DB8317, 0x1C291D04
	.long 0x0D3B6092, 0x9E4ADDF8
	.long 0xC96CFDC0, 0x740EEF02
	.long 0x878A92A7, 0x39D3B296
	.long 0xDAECE73E, 0x083A6EEC
	.long 0xAB7AFF2A, 0x0715CE53
	.long 0x2162D385, 0xC49F4F67
	.long 0x83348832, 0x47DB8317
	.long 0x299847D5, 0x2AD91C30
	.long 0xB9E02B86, 0x0D3B6092
	.long 0x18B33A4E, 0x6992CEA2
	.long 0xB6DD949B, 0xC96CFDC0
	.long 0x78D9CCB7, 0x7E908048
	.long 0xBAC2FD7B, 0x878A92A7
	.long 0xA60CE07B, 0x1B3D8F29
	.long 0xCE7F39F4, 0xDAECE73E
	.long 0x61D82E56, 0xF1D0F55E
	.long 0xD270F1A2, 0xAB7AFF2A
	.long 0xC619809D, 0xA87AB8A8
	.long 0x2B3CAC5D, 0x2162D385
	.long 0x65863B64, 0x8462D800
	.long 0x1B03397F, 0x83348832
	.long 0xEBB883BD, 0x71D111A8
	.long 0xB3E32C28, 0x299847D5
	.long 0x064F7F26, 0xFFD852C6
	.long 0xDD7E3B0C, 0xB9E02B86
	.long 0xF285651C, 0xDCB17AA4
	.long 0x10746F3C, 0x18B33A4E
	.long 0xC7A68855, 0xF37C5AEE
	.long 0x271D9844, 0xB6DD949B
	.long 0x8E766A0C, 0x6051D5A2
	.long 0x93A5F730, 0x78D9CCB7
	.long 0x6CB08E5C, 0x18B0D4FF
	.long 0x6B749FB2, 0xBAC2FD7B
	.long 0x1393E203, 0x21F3D99C
	.long 0xCEC3662E, 0xA60CE07B
	.long 0x96C515BB, 0x8F158014
	.long 0xE6FC4E6A, 0xCE7F39F4
	.long 0x8227BB8A, 0xA00457F7
	.long 0xB0CD4768, 0x61D82E56
	.long 0x39C7FF35, 0x8D6D2C43
	.long 0xD7A4825C, 0xD270F1A2
	.long 0x0AB3844B, 0x00AC29CF
	.long 0x0167D312, 0xC619809D
	.long 0xF6076544, 0xE9ADF796
	.long 0x26F6A60A, 0x2B3CAC5D
	.long 0xA741C1BF, 0x96638B34
	.long 0x98D8D9CB, 0x65863B64
	.long 0x49C3CC9C, 0xE0E9F351
	.long 0x68BCE87A, 0x1B03397F
	.long 0x57A3D037, 0x9AF01F2D
	.long 0x6956FC3B, 0xEBB883BD
	.long 0x42D98888, 0x2CFF42CF
	.long 0x3771E98F, 0xB3E32C28
	.long 0xB42AE3D9, 0x88F25A3A
	.long 0x2178513A, 0x064F7F26
	.long 0xE0AC139E, 0x4E36F0B0
	.long 0x170076FA, 0xDD7E3B0C
	.long 0x444DD413, 0xBD6F81F8
	.long 0x6F345E45, 0xF285651C
	.long 0x41D17B64, 0x91C9BD4B
	.long 0xFF0DBA97, 0x10746F3C
	.long 0xA2B73DF1, 0x885F087B
	.long 0xF872E54C, 0xC7A68855
	.long 0x1E41E9FC, 0x4C144932
	.long 0x86D8E4D2, 0x271D9844
	.long 0x651BD98B, 0x52148F02
	.long 0x5BB8F1BC, 0x8E766A0C
	.long 0xA90FD27A, 0xA3C6F37A
	.long 0xB3AF077A, 0x93A5F730
	.long 0x4984D782, 0xD7C0557F
	.long 0xCA6EF3AC, 0x6CB08E5C
	.long 0x234E0B26, 0x63DED06A
	.long 0xDD66CBBB, 0x6B749FB2
	.long 0x4597456A, 0x4D56973C
	.long 0xE9E28EB4, 0x1393E203
	.long 0x7B3FF57A, 0x9669C9DF
	.long 0xC9C8B782, 0xCEC3662E
	.long 0x3F70CC6F, 0xE417F38A
	.long 0x93E106A4, 0x96C515BB
	.long 0x62EC6C6D, 0x4B9E0F71
	.long 0xD813B325, 0xE6FC4E6A
	.long 0x0DF04680, 0xD104B8FC
	.long 0x2342001E, 0x8227BB8A
	.long 0x0A2A8D7E, 0x5B397730
	.long 0x6D9A4957, 0xB0CD4768
	.long 0xE8B6368B, 0xE78EB416
	.long 0xD2C3ED1A, 0x39C7FF35
	.long 0x995A5724, 0x61FF0E01
	.long 0x9EF68D35, 0xD7A4825C
	.long 0x0C139B31, 0x8D96551C
	.long 0xF2271E60, 0x0AB3844B
	.long 0x0B0BF8CA, 0x0BF80DD2
	.long 0x2664FD8B, 0x0167D312
	.long 0xED64812D, 0x8821ABED
	.long 0x02EE03B2, 0xF6076544
	.long 0x8604AE0F, 0x6A45D2B2
	.long 0x363BD6B3, 0x26F6A60A
	.long 0x135C83FD, 0xD8D26619
	.long 0x5FABE670, 0xA741C1BF
	.long 0x35EC3279, 0xDE87806C
	.long 0x00BCF5F6, 0x98D8D9CB
	.long 0x8AE00689, 0x14338754
	.long 0x17F27698, 0x49C3CC9C
	.long 0x58CA5F00, 0x5BD2011F
	.long 0xAA7C7AD5, 0x68BCE87A
	.long 0xB5CFCA28, 0xDD07448E
	.long 0xDED288F8, 0x57A3D037
	.long 0x59F229BC, 0xDDE8F5B9
	.long 0x6D390DEC, 0x6956FC3B
	.long 0x37170390, 0xA3E3E02C
	.long 0x6353C1CC, 0x42D98888
	.long 0xC4584F5C, 0xD73C7BEA
	.long 0xF48642E9, 0x3771E98F
	.long 0x531377E2, 0x80FF0093
	.long 0xDD35BC8D, 0xB42AE3D9
	.long 0xB25B29F2, 0x8FE4C34D
	.long 0x9A5EDE41, 0x2178513A
	.long 0xA563905D, 0xDF99FC11
	.long 0x45CDDF4E, 0xE0AC139E
	.long 0xACFA3103, 0x6C23E841
	.long 0xA51B6135, 0x170076FA
    .size K_table, .-K_table
