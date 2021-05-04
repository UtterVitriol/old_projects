	.intel_syntax noprefix
	.globl	main
	.type	main, @function

main:
	endbr64
	xor	rax, rax
	push	rbp
	mov	rbp, rsp
	sub	rsp, 128

	cmp	rdi, 3		# check for too many args
	jg	printUsage
	
	cmp	rdi, 1		# check for no args
	je	promptUser

	mov	BYTE PTR -33[rbp], 0	# set -o flag to false
	mov	BYTE PTR -34 [rbp], 0	# set got int flag to false

	jmp	checkArgs

	checkedArgs:

	cmp	BYTE PTR -34[rbp], 1	# check for got int flag
	jl	promptUser

	gotInput:
	
	mov	edi, DWORD PTR -4[rbp]	# mov result from getArg to rdi

	lea	rsi, QWORD PTR -32[rbp]	# reserve space for big num

	call	fast_fibonacci

	cmp	BYTE PTR -33[rbp], 0	# check for -o flag
	je	printHex
	jmp	printOctal

getArgInt:
	endbr64
	push	rbp
	mov	rsi, 0
	mov	rdx, 10
	call	strtol
	pop	rbp
	ret

checkArgs:
	mov	BYTE PTR -35[rbp], dil	# save argc
	dec	BYTE PTR -35[rbp]	# decrement argc

argLoop:
	cmp	BYTE PTR -35[rbp], 0	# check if all args have been parsed
	jle	checkedArgs

	add	rsi, 8			# move argv ptr
	mov	rdi, QWORD PTR [rsi]	# get what's in argv ptr
	movsx	edx, BYTE PTR [rdi]	# get first char of ptr

	# check for -o option
	cmp	rdx, '-'		# check for '-'
	jne	notTac			# not '-'

	cmp	BYTE PTR -33[rbp], 1	# check -o flag
	je	printUsage

	inc	rdi			# move to next char
	movsx	edx, BYTE PTR [rdi]	# get next char
	cmp	rdx, 'o'		# check for 'o'
	jne	printUsage

	mov	BYTE PTR -33[rbp], 1	# set -o flag

	dec	BYTE PTR -35[rbp]	# decrement loop counter
	jmp	argLoop

notTac:
	# check for integer
	cmp	edx, '0'		# check for negative number (not sure if possible)
	jl	printUsage
	cmp	edx, '9'		# check for anything above 9
	jg	printUsage

	cmp	BYTE PTR -34[rbp], 1	# check got int flag
	je	printUsage

	push	rsi			# preserve rsi
	call	getArgInt		# get integer
	pop	rsi			# restore rsi

	cmp	rax, 300		# check for int greater than 300
	jg	printUsage

	mov	DWORD PTR -4[rbp], eax	# save integer

	mov	BYTE PTR -34[rbp], 1	# set got int flag

	dec	BYTE PTR -35[rbp]	# decrement loop counter
	jmp	argLoop


printHex:
	# print result of fast_fibonacci in hex

	# print "0x"
	lea	rdi, hexprf[rip]
	call	printf

	# check and print if data in most significant qword
	mov	rsi, QWORD PTR -32[rbp]
	cmp	rsi, 0
	je	hskip1
	lea	rdi, hexmid[rip]
	call	printf

	# check and print if data in second most significant qword
	hskip1:
	mov	rsi, QWORD PTR -24[rbp]
	cmp	rsi, 0
	je	hskip2
	lea	rdi, hexmid[rip]
	call	printf

	# check and print if data in third most significant qword
	hskip2:
	mov	rsi, QWORD PTR -16[rbp]
	cmp	rsi, 0
	je	hskip3
	lea	rdi, hexmid[rip]
	call	printf

	# check and print if data in lest significant qword
	hskip3:
	mov	rsi, QWORD PTR -8[rbp]
	lea	rdi, hexmid[rip]
	call	printf

	# print newline
	lea	rdi, newline[rip]
	call	printf

	mov	eax, 0

exit:
	leave
	ret

printOctal:
	# print result of fast_fibonacci in octal
	# realigns bits

	# print "0o"
	lea	rdi, octprf[rip]
	call	printf

	lea	rax, octend[rip]
	mov	QWORD PTR -128[rbp], rax

	# check and print if data in most significant qword
	mov	rsi, QWORD PTR -32[rbp]
	cmp	rsi, 0
	je	skip1
	mov	rdi, QWORD PTR -128[rbp]
	call	printf
	
	lea	rax, octmid[rip]
	mov	QWORD PTR -128[rbp], rax
	# check and print if data in second most significant qword
	skip1:
	mov	rsi, QWORD PTR -24[rbp]

	mov	rax, 0x1			# mask for last bit
	and	rax, rsi			# get last bit
	mov	QWORD PTR -40[rbp], rax		# save bit for later

	shr	rsi, 1				# shift right one bit to realign

	cmp	rsi, 0
	je	skip2
	mov	rdi, QWORD PTR -128[rbp]
	call	printf

	lea	rax, octmid[rip]
	mov	QWORD PTR -128[rbp], rax

	# check and print if data in third most significant qword
	skip2:
	mov	rsi, QWORD PTR -16[rbp]

	mov	rdx, 0x3			# mask for last two bits
	and	rdx, rsi			# get last two bits
	mov	QWORD PTR -48[rbp], rdx		# save fore later

	mov	rax, QWORD PTR -40[rbp]		# get bit from previous section
	shl	rax, 62				# move bit to correct postion

	shr	rsi, 2				# shift right 2 bits to realign
	or	rsi, rax			# flip bit from last section

	cmp	rsi, 0
	je	skip3
	mov	rdi, QWORD PTR -128[rbp]
	call	printf
	
	lea	rax, octmid[rip]
	mov	QWORD PTR -128[rbp], rax

	# check and print if data in fourth most significant qword
	skip3:
	mov	rsi, QWORD PTR -8[rbp]

	mov	rax, 0x7			# mask for last three bits
	and	rax, rsi			# get last three bits
	mov	QWORD PTR -40[rbp], rax		# save for later

	mov	rdx, QWORD PTR -48[rbp]		# get the two bits from last section
	shl	rdx, 61				# move bits to correct position

	shr	rsi, 3				# shift right to realign
	or	rsi, rdx			# flip bits from last section

	mov	rdi, QWORD PTR -128[rbp]
	call	printf

	# 3 least significant bits
	mov	rsi, QWORD PTR -40[rbp]		# get last three bits
	lea	rdi, octend[rip]
	call	printf

	# print newline
	lea	rdi, newline[rip]
	call	printf
	
	mov	eax, 0
	jmp	exit

printUsage:
	xor	rcx, rcx	# make printf happy
	xor	rax, rax	# make printf happy
	xor	rsi, rsi	# make printf happy
	xor	rdx, rdx	# make printf happy

	lea	rdi, usage[rip]
	call	printf
	mov	eax, 1
	jmp	exit

promptUser:
	# get user input

	lea	rdi, userPrompt[rip]		# print prompt
	call	printf

	mov	rdx, QWORD PTR stdin[rip]	# stdin
	lea	rax, QWORD PTR -4[rbp]		# bytes to write to
	mov	esi, 4				# how many bytes to read
	mov	rdi, rax			# putting the address right into rdi seemed to cause a segfault
	call	fgets

	lea	rax, QWORD PTR -4[rbp]		# bytes from fgets
	mov	rsi, 0				# NULL
	mov	rdx, 10				# base 10
	mov	rdi, rax			# same as last time
	call	strtol

	cmp	eax, 0				# check for negative (not sure if possible)
	jl	printUsage
	cmp	eax, 300			# check for > 300
	jg	printUsage

	mov	DWORD PTR -4[rbp], eax		# save int

	jmp	gotInput

fast_fibonacci:
	# Written by: Saugat Bhattarai

	xor	rax, rax
	xor	rdx, rdx
	xor	r8, r8
	xor	r9, r9

	# Short circuit for F(0)
	cmp	rdi, 0		# F(0) is 0...
	je	done		# ... so we're done

	# preserve r12-r15 in the red zone
	mov	[rsp-8], r12
	mov	[rsp-16], r13
	mov	[rsp-24], r14
	mov	[rsp-32], r15
	
	# Intitialize F(1) to 1
	mov	r12, 1
	xor	r13, r13
	xor	r14, r14
	xor	r15, r15

	mov	rcx,rdi
	
next:
	xadd	rax,r12	
	adc	r13, 0	
	xadd	rdx,r13	
	adc	r14, 0	
	xadd	r8,r14	
	adc	r15, 0	
	xadd	r9,r15	
	loop	next	

	# restore r12-r15
	mov	r12, [rsp-8]
	mov	r13, [rsp-16]
	mov	r14, [rsp-24]
	mov	r15, [rsp-32]
	
done:	# move 256 bit number to buffer in 255..0 order
	mov	QWORD PTR [rsi+0], r9
	mov	QWORD PTR [rsi+8], r8
	mov	QWORD PTR [rsi+16], rdx
	mov	QWORD PTR [rsi+24], rax
	ret

userPrompt:
	.string "Enter a number (0 - 300): "
octprf:
	.string "0o"
octend:
	.string "%lo"
octmid:
	.string "%021lo"
hexprf:
	.string "0x"
hexmid:
	.string "%0lX"
newline:
	.string "\n"
usage:
	.string "Error:\n./fibonacci [-o] <num(0-300)>\n-o - print fibonacci number in octal\n"
