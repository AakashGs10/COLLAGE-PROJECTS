import struct

# ========== Pure Python SHA-256 Implementation (no hashlib) ==========
# (This is a simplified version for educational purposes)

def right_rotate(value, shift):
    return ((value >> shift) | (value << (32 - shift))) & 0xffffffff

def sha256(message: bytes) -> str:
    # Initialize hash values:
    h = [
        0x6a09e667,
        0xbb67ae85,
        0x3c6ef372,
        0xa54ff53a,
        0x510e527f,
        0x9b05688c,
        0x1f83d9ab,
        0x5be0cd19,
    ]

    # Initialize array of round constants:
    k = [
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
    ]

    # Pre-processing
    original_len_in_bits = (8 * len(message)) & 0xffffffffffffffff
    message += b'\x80'
    while (len(message) * 8) % 512 != 448:
        message += b'\x00'
    message += struct.pack('>Q', original_len_in_bits)

    # Process the message in successive 512-bit chunks# ========== Pure Python SHA-256 Implementation (no hashlib, no struct) ==========

def right_rotate(value, shift):
    return ((value >> shift) | (value << (32 - shift))) & 0xffffffff

def sha256(message: bytes) -> str:
    # Initialize hash values
    h = [
        0x6a09e667,
        0xbb67ae85,
        0x3c6ef372,
        0xa54ff53a,
        0x510e527f,
        0x9b05688c,
        0x1f83d9ab,
        0x5be0cd19,
    ]

    # Round constants
    k = [
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
    ]

    # Pre-processing (padding)
    original_len_in_bits = (8 * len(message)) & 0xffffffffffffffff
    message += b'\x80'
    while (len(message) * 8) % 512 != 448:
        message += b'\x00'
    message += original_len_in_bits.to_bytes(8, 'big')

    # Process message in 512-bit chunks
    for chunk_start in range(0, len(message), 64):
        chunk = message[chunk_start:chunk_start + 64]

        # Break chunk into 16 words of 32 bits
        w = [int.from_bytes(chunk[i:i+4], 'big') for i in range(0, 64, 4)] + [0]*48

        # Extend to 64 words
        for i in range(16, 64):
            s0 = right_rotate(w[i-15], 7) ^ right_rotate(w[i-15], 18) ^ (w[i-15] >> 3)
            s1 = right_rotate(w[i-2], 17) ^ right_rotate(w[i-2], 19) ^ (w[i-2] >> 10)
            w[i] = (w[i-16] + s0 + w[i-7] + s1) & 0xffffffff

        a, b, c, d, e, f, g, h0 = h
        for i in range(64):
            s1 = right_rotate(e, 6) ^ right_rotate(e, 11) ^ right_rotate(e, 25)
            ch = (e & f) ^ ((~e) & g)
            temp1 = (h0 + s1 + ch + k[i] + w[i]) & 0xffffffff
            s0 = right_rotate(a, 2) ^ right_rotate(a, 13) ^ right_rotate(a, 22)
            maj = (a & b) ^ (a & c) ^ (b & c)
            temp2 = (s0 + maj) & 0xffffffff

            h0 = g
            g = f
            f = e
            e = (d + temp1) & 0xffffffff
            d = c
            c = b
            b = a
            a = (temp1 + temp2) & 0xffffffff

        h = [(x+y) & 0xffffffff for x,y in zip(h, [a,b,c,d,e,f,g,h0])]

    return ''.join(f'{x:08x}' for x in h)


# ========== Merkle Tree ==========

def build_merkle_root(transactions):
    layer = [sha256(tx.encode()) for tx in transactions]
    print("Leaf Hashes:")
    for i, h in enumerate(layer):
        print(f"TX{i+1}: {h}")

    while len(layer) > 1:
        new_layer = []
        for i in range(0, len(layer), 2):
            left = layer[i]
            right = layer[i+1] if i+1 < len(layer) else layer[i]
            combined = sha256((left + right).encode())
            new_layer.append(combined)
            print(f"Hash({left[:8]} + {right[:8]}) = {combined}")
        layer = new_layer

    return layer[0]


# ========== Main Program ==========

if __name__ == "__main__":
    with open("transactions_20.csv", "r") as f:
        lines = f.read().strip().split("\n")[1:]  # skip header
        txs = [line.replace(",", "|") for line in lines]

    root = build_merkle_root(txs)
    print("\nMerkle Root:", root)

    for chunk_start in range(0, len(message), 64):
        chunk = message[chunk_start:chunk_start + 64]
        w = list(struct.unpack('>16L', chunk)) + [0]*48
        for i in range(16, 64):
            s0 = right_rotate(w[i-15], 7) ^ right_rotate(w[i-15], 18) ^ (w[i-15] >> 3)
            s1 = right_rotate(w[i-2], 17) ^ right_rotate(w[i-2], 19) ^ (w[i-2] >> 10)
            w[i] = (w[i-16] + s0 + w[i-7] + s1) & 0xffffffff

        a, b, c, d, e, f, g, h0 = h
        for i in range(64):
            s1 = right_rotate(e, 6) ^ right_rotate(e, 11) ^ right_rotate(e, 25)
            ch = (e & f) ^ ((~e) & g)
            temp1 = (h0 + s1 + ch + k[i] + w[i]) & 0xffffffff
            s0 = right_rotate(a, 2) ^ right_rotate(a, 13) ^ right_rotate(a, 22)
            maj = (a & b) ^ (a & c) ^ (b & c)
            temp2 = (s0 + maj) & 0xffffffff

            h0 = g
            g = f
            f = e
            e = (d + temp1) & 0xffffffff
            d = c
            c = b
            b = a
            a = (temp1 + temp2) & 0xffffffff

        h = [(x+y) & 0xffffffff for x,y in zip(h, [a,b,c,d,e,f,g,h0])]

    return ''.join(f'{x:08x}' for x in h)

# ========== Merkle Tree ==========

def build_merkle_root(transactions):
    # Hash each transaction
    layer = [sha256(tx.encode()) for tx in transactions]
    print("Leaf Hashes:")
    for i, h in enumerate(layer):
        print(f"TX{i+1}: {h}")
    
    # Build up tree
    while len(layer) > 1:
        new_layer = []
        for i in range(0, len(layer), 2):
            left = layer[i]
            right = layer[i+1] if i+1 < len(layer) else layer[i]  # duplicate if odd
            combined = sha256((left + right).encode())
            new_layer.append(combined)
            print(f"Hash({left[:8]} + {right[:8]}) = {combined}")
        layer = new_layer
    
    return layer[0]

# ========== Main Program ==========
if _name_ == "_main_":
    # Read transactions from file (transactions.txt with one TX per line)
    with open("transactions.txt", "r") as f:
        txs = [line.strip() for line in f if line.strip()]

    root = build_merkle_root(txs)
    print("\nMerkle Root:", root)
