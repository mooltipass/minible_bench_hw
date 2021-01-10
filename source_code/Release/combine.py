from intelhex import IntelHex
one = IntelHex("Mooltipass.hex")
two = IntelHex("bootloader_mini.hex")
one.merge(two)
one.tofile("comb.hex", format="hex")