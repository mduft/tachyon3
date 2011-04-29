target remote :1234

define si
    stepi
    disassemble $pc,$pc+0x10
end

define nx
    next
    disassemble $pc,$pc+0x10
end
