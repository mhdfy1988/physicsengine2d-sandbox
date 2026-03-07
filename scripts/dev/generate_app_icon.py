#!/usr/bin/env python3
import math
import os
import struct
import zlib


def clamp(v, lo, hi):
    return lo if v < lo else hi if v > hi else v


def smoothstep(e0, e1, x):
    if e0 == e1:
        return 1.0 if x >= e1 else 0.0
    t = clamp((x - e0) / (e1 - e0), 0.0, 1.0)
    return t * t * (3.0 - 2.0 * t)


def lerp(a, b, t):
    return a + (b - a) * t


def mix_color(c0, c1, t):
    return (
        lerp(c0[0], c1[0], t),
        lerp(c0[1], c1[1], t),
        lerp(c0[2], c1[2], t),
        lerp(c0[3], c1[3], t),
    )


def alpha_blend(dst, src):
    sr, sg, sb, sa = src
    dr, dg, db, da = dst
    out_a = sa + da * (1.0 - sa)
    if out_a <= 1e-6:
        return (0.0, 0.0, 0.0, 0.0)
    out_r = (sr * sa + dr * da * (1.0 - sa)) / out_a
    out_g = (sg * sa + dg * da * (1.0 - sa)) / out_a
    out_b = (sb * sa + db * da * (1.0 - sa)) / out_a
    return (out_r, out_g, out_b, out_a)


def sd_rounded_rect(px, py, cx, cy, hw, hh, r):
    qx = abs(px - cx) - hw + r
    qy = abs(py - cy) - hh + r
    ox = max(qx, 0.0)
    oy = max(qy, 0.0)
    outside = math.hypot(ox, oy)
    inside = min(max(qx, qy), 0.0)
    return outside + inside - r


def circle_fill_alpha(px, py, cx, cy, radius, aa):
    d = math.hypot(px - cx, py - cy) - radius
    return 1.0 - smoothstep(-aa, aa, d)


def circle_stroke_alpha(px, py, cx, cy, radius, width, aa):
    d = abs(math.hypot(px - cx, py - cy) - radius) - (width * 0.5)
    return 1.0 - smoothstep(-aa, aa, d)


def to_u8(v):
    return int(clamp(v * 255.0 + 0.5, 0.0, 255.0))


def render_icon(size):
    w = size
    h = size
    aa = max(0.55, size / 256.0)

    ring_outer = (0.36, 0.71, 0.93, 0.80)
    ring_inner = (0.70, 0.87, 0.99, 0.70)
    core = (1.00, 0.63, 0.20, 1.0)
    core_glow = (1.00, 0.84, 0.45, 0.95)
    node_a = (0.52, 0.87, 1.00, 1.0)
    node_b = (1.00, 0.75, 0.35, 1.0)
    node_c = (0.78, 0.93, 1.00, 1.0)

    cx = w * 0.5
    cy = h * 0.5
    orbit1_r = w * 0.22
    orbit2_r = w * 0.31
    orbit_w = max(1.3, w * 0.030)

    core_r = w * 0.085
    core_ring_r = w * 0.13
    core_ring_w = max(1.1, w * 0.022)

    node_r = max(1.3, w * 0.048)

    pixels = bytearray(w * h * 4)
    for y in range(h):
        for x in range(w):
            px = x + 0.5
            py = y + 0.5
            idx = (y * w + x) * 4

            out = (0.0, 0.0, 0.0, 0.0)

            a_orbit2 = circle_stroke_alpha(px, py, cx, cy, orbit2_r, orbit_w, aa) * 0.72
            if a_orbit2 > 0.0:
                out = alpha_blend(out, (ring_outer[0], ring_outer[1], ring_outer[2], a_orbit2))

            a_orbit1 = circle_stroke_alpha(px, py, cx, cy, orbit1_r, orbit_w, aa) * 0.85
            if a_orbit1 > 0.0:
                out = alpha_blend(out, (ring_inner[0], ring_inner[1], ring_inner[2], a_orbit1))

            a_core_ring = circle_stroke_alpha(px, py, cx, cy, core_ring_r, core_ring_w, aa) * 0.85
            if a_core_ring > 0.0:
                out = alpha_blend(out, (0.95, 0.98, 1.0, a_core_ring))

            a_core_glow = circle_fill_alpha(px, py, cx, cy, core_r * 1.34, aa) * 0.18
            if a_core_glow > 0.0:
                out = alpha_blend(out, (core_glow[0], core_glow[1], core_glow[2], a_core_glow))

            a_core = circle_fill_alpha(px, py, cx, cy, core_r, aa)
            if a_core > 0.0:
                out = alpha_blend(out, (core[0], core[1], core[2], a_core))

            theta = [-0.75, 1.58, 3.62]
            radii = [orbit2_r, orbit1_r, orbit2_r]
            colors = [node_a, node_b, node_c]
            for i in range(3):
                nx = cx + radii[i] * math.cos(theta[i])
                ny = cy + radii[i] * math.sin(theta[i])
                a_node = circle_fill_alpha(px, py, nx, ny, node_r, aa)
                if a_node > 0.0:
                    out = alpha_blend(out, (colors[i][0], colors[i][1], colors[i][2], a_node))

            pixels[idx + 0] = to_u8(out[0])
            pixels[idx + 1] = to_u8(out[1])
            pixels[idx + 2] = to_u8(out[2])
            pixels[idx + 3] = to_u8(out[3])

    return bytes(pixels)


def png_chunk(chunk_type, data):
    crc = zlib.crc32(chunk_type + data) & 0xFFFFFFFF
    return struct.pack(">I", len(data)) + chunk_type + data + struct.pack(">I", crc)


def encode_png_rgba(size, rgba_bytes):
    raw = bytearray()
    stride = size * 4
    for y in range(size):
        raw.append(0)
        start = y * stride
        raw.extend(rgba_bytes[start:start + stride])

    ihdr = struct.pack(">IIBBBBB", size, size, 8, 6, 0, 0, 0)
    idat = zlib.compress(bytes(raw), level=9)
    return b"\x89PNG\r\n\x1a\n" + png_chunk(b"IHDR", ihdr) + png_chunk(b"IDAT", idat) + png_chunk(b"IEND", b"")


def write_ico(path, png_images):
    count = len(png_images)
    header = struct.pack("<HHH", 0, 1, count)
    entries = bytearray()
    data_blob = bytearray()
    offset = 6 + (16 * count)

    for size, png_data in png_images:
        width_byte = 0 if size >= 256 else size
        height_byte = 0 if size >= 256 else size
        entry = struct.pack(
            "<BBBBHHII",
            width_byte,
            height_byte,
            0,
            0,
            1,
            32,
            len(png_data),
            offset,
        )
        entries.extend(entry)
        data_blob.extend(png_data)
        offset += len(png_data)

    with open(path, "wb") as f:
        f.write(header)
        f.write(entries)
        f.write(data_blob)


def write_svg(path):
    svg = """<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512">
  <defs/>
  <circle cx="256" cy="256" r="160" fill="none" stroke="#63b6ec" stroke-opacity="0.72" stroke-width="15"/>
  <circle cx="256" cy="256" r="112" fill="none" stroke="#b5e0ff" stroke-opacity="0.88" stroke-width="15"/>
  <circle cx="256" cy="256" r="42" fill="#ffa233"/>
  <circle cx="256" cy="256" r="66" fill="none" stroke="#eef8ff" stroke-width="11"/>
  <circle cx="373" cy="148" r="24" fill="#89dcff"/>
  <circle cx="255" cy="368" r="24" fill="#ffc269"/>
  <circle cx="112" cy="198" r="24" fill="#c5ecff"/>
</svg>
"""
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(svg)


def main():
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    out_dir = os.path.join(repo_root, "assets", "icons")
    os.makedirs(out_dir, exist_ok=True)

    sizes = [16, 24, 32, 48, 64, 128, 256]
    png_images = []
    for s in sizes:
        rgba = render_icon(s)
        png = encode_png_rgba(s, rgba)
        png_images.append((s, png))
        with open(os.path.join(out_dir, f"physics_sandbox_{s}.png"), "wb") as f:
            f.write(png)

    write_ico(os.path.join(out_dir, "physics_sandbox.ico"), png_images)
    write_svg(os.path.join(out_dir, "physics_sandbox.svg"))
    print("Generated icon assets in assets/icons/")


if __name__ == "__main__":
    main()
