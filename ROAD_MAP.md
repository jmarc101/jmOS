## Next Steps

### Recommended Order

1. **GDT (Global Descriptor Table)**
   - Set up your own segments instead of relying on bootloader's
   - Understand protected mode segmentation

2. **IDT (Interrupt Descriptor Table)**
   - Handle CPU exceptions (divide by zero, page faults, etc.)
   - Foundation for everything else

3. **PIT Timer (Programmable Interval Timer)**
   - First hardware interrupt (IRQ0)
   - Gives you a heartbeat/tick

4. **Keyboard Driver**
   - PS/2 keyboard via IRQ1
   - Now you can actually interact with your OS

5. **Simple Shell**
   - Read input, parse commands, execute
   - Tie it all together

### Why This Order

```
GDT → IDT → PIT → Keyboard → Shell
         ↓
    (interrupts required)
```

You can't handle keyboard or timer without interrupts. You need GDT set up properly before IDT.

### Resources

- [OSDev Wiki - GDT](https://wiki.osdev.org/GDT)
- [OSDev Wiki - IDT](https://wiki.osdev.org/IDT)
- James Molloy's kernel tutorials

---
