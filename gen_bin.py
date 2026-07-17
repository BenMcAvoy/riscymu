import sys
import struct

def main():
    if len(sys.argv) > 1:
        lines = open(sys.argv[1]).read().split()
    else:
        lines = sys.stdin.read().split()

    lines_wrote = 0

    with open("program.bin", "wb") as f:
        for line in lines:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if line.startswith("0x"):
                line = line[2:]
            val = int(line, 16)
            f.write(struct.pack("<I", val))
            lines_wrote += 1

    print(f"wrote {lines_wrote} instrs to program.bin")

if __name__ == "__main__":
    main()
