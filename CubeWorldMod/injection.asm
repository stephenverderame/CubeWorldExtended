.MODEL flat, stdcall
PUBLIC petInventoryInjection
EXTERN puts:proc
.DATA
	variable1 dd 0
.CODE
petInventoryInjection proc
	mov [variable1], edi

	mov eax, [edi]
	lea esi, [edi + 4]

	jmp variable1
petInventoryInjection endp
END