[bits 32]

; paging.asm - Assembly functions for virtual memory management

section .text

; Load page directory into CR3
global vmm_load_page_directory
vmm_load_page_directory:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]  ; Get page directory physical address
    mov cr3, eax        ; Load into CR3
    
    pop ebp
    ret

; Enable paging by setting PG bit in CR0
global vmm_enable_paging_asm
vmm_enable_paging_asm:
    push ebp
    mov ebp, esp
    
    mov eax, cr0
    or eax, 0x80000000  ; Set PG (paging) bit
    mov cr0, eax
    
    pop ebp
    ret

; Flush TLB by reloading CR3
global vmm_flush_tlb
vmm_flush_tlb:
    push ebp
    mov ebp, esp
    
    mov eax, cr3
    mov cr3, eax        ; Reload CR3 to flush TLB
    
    pop ebp
    ret

; Get current page directory from CR3
global vmm_get_page_directory
vmm_get_page_directory:
    mov eax, cr3
    ret

; Invalidate specific page in TLB
global vmm_invalidate_page
vmm_invalidate_page:
    push ebp
    mov ebp, esp
    
    mov eax, [ebp + 8]  ; Get virtual address
    invlpg [eax]        ; Invalidate page in TLB
    
    pop ebp
    ret 