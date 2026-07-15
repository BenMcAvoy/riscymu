import sys
import struct

def main():
    if len(sys.argv) > 1:
        lines = open(sys.argv[1]).read().split()
    else:
        lines = sys.stdin.read().split()

    with open("program.bin", "wb") as f:
        for line in lines:
            line = line.strip()
            if not line:
                continue
            val = int(line, 16)
            f.write(struct.pack("<I", val))

    print(f"wrote {len(lines)} instrs to program.bin")

if __name__ == "__main__":
    main()
