#!/usr/bin/env python3
"""Generate tests/assets/skinned_bar.gltf (and .glb).

A two-joint bar, skinned and animated, small enough to commit and specific
enough to assert on. The bar stands along +Y from y=0 to y=2:

  - joint 0 sits at the origin and never moves; the bottom four vertices are
    weighted entirely to it.
  - joint 1 sits at y=1 and rotates 90 degrees about +Z over one second; the
    top four vertices are weighted entirely to it.

So at t=0 the pose is the bind pose, and at t=1 the top of the bar has swung
to -X. tests/gltf_animation_test.cpp checks exactly that.
"""

import base64
import json
import math
import pathlib
import struct

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT_DIR = ROOT / "tests/assets"

HALF = 0.25
HEIGHT = 2.0

POSITIONS = [
    (-HALF, 0.0, -HALF), (HALF, 0.0, -HALF), (HALF, 0.0, HALF), (-HALF, 0.0, HALF),
    (-HALF, HEIGHT, -HALF), (HALF, HEIGHT, -HALF), (HALF, HEIGHT, HALF), (-HALF, HEIGHT, HALF),
]

# bottom ring -> joint 0, top ring -> joint 1
JOINTS = [(0, 0, 0, 0)] * 4 + [(1, 0, 0, 0)] * 4
WEIGHTS = [(1.0, 0.0, 0.0, 0.0)] * 8

INDICES = [
    0, 1, 2, 0, 2, 3,        # bottom
    4, 6, 5, 4, 7, 6,        # top
    0, 4, 5, 0, 5, 1,        # -Z
    1, 5, 6, 1, 6, 2,        # +X
    2, 6, 7, 2, 7, 3,        # +Z
    3, 7, 4, 3, 4, 0,        # -X
]

# column-major, as glTF stores matrices
IDENTITY = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1]
TRANSLATE_DOWN_1 = [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, -1, 0, 1]

KEY_TIMES = [0.0, 0.5, 1.0]
KEY_ANGLES = [0.0, math.pi / 4.0, math.pi / 2.0]


def pad_to_4(blob: bytes) -> bytes:
    return blob + b"\0" * (-len(blob) % 4)


def build():
    chunks = []
    views = []
    offset = 0

    def add(blob: bytes, target=None) -> int:
        nonlocal offset
        blob = pad_to_4(blob)
        view = {"buffer": 0, "byteOffset": offset, "byteLength": len(blob)}
        if target is not None:
            view["target"] = target
        views.append(view)
        chunks.append(blob)
        offset += len(blob)
        return len(views) - 1

    position_view = add(b"".join(struct.pack("<3f", *p) for p in POSITIONS), 34962)
    joint_view = add(b"".join(struct.pack("<4H", *j) for j in JOINTS), 34962)
    weight_view = add(b"".join(struct.pack("<4f", *w) for w in WEIGHTS), 34962)
    index_view = add(struct.pack(f"<{len(INDICES)}H", *INDICES), 34963)
    ibm_view = add(struct.pack("<16f", *IDENTITY) + struct.pack("<16f", *TRANSLATE_DOWN_1))
    time_view = add(struct.pack(f"<{len(KEY_TIMES)}f", *KEY_TIMES))
    rotation_view = add(b"".join(
        struct.pack("<4f", 0.0, 0.0, math.sin(a / 2.0), math.cos(a / 2.0)) for a in KEY_ANGLES
    ))

    buffer_blob = b"".join(chunks)

    minimums = [min(p[i] for p in POSITIONS) for i in range(3)]
    maximums = [max(p[i] for p in POSITIONS) for i in range(3)]

    accessors = [
        {"bufferView": position_view, "componentType": 5126, "count": len(POSITIONS),
         "type": "VEC3", "min": minimums, "max": maximums},
        {"bufferView": joint_view, "componentType": 5123, "count": len(JOINTS), "type": "VEC4"},
        {"bufferView": weight_view, "componentType": 5126, "count": len(WEIGHTS), "type": "VEC4"},
        {"bufferView": index_view, "componentType": 5123, "count": len(INDICES), "type": "SCALAR"},
        {"bufferView": ibm_view, "componentType": 5126, "count": 2, "type": "MAT4"},
        {"bufferView": time_view, "componentType": 5126, "count": len(KEY_TIMES), "type": "SCALAR",
         "min": [KEY_TIMES[0]], "max": [KEY_TIMES[-1]]},
        {"bufferView": rotation_view, "componentType": 5126, "count": len(KEY_ANGLES), "type": "VEC4"},
    ]

    gltf = {
        "asset": {"version": "2.0", "generator": "nle tools/make_test_gltf.py"},
        "scene": 0,
        "scenes": [{"name": "skinned_bar", "nodes": [0, 1]}],
        "nodes": [
            {"name": "bar", "mesh": 0, "skin": 0},
            {"name": "joint_root", "children": [2]},
            {"name": "joint_tip", "translation": [0.0, 1.0, 0.0]},
        ],
        "meshes": [{
            "name": "bar_mesh",
            "primitives": [{
                "attributes": {"POSITION": 0, "JOINTS_0": 1, "WEIGHTS_0": 2},
                "indices": 3,
                "material": 0,
                "mode": 4,
            }],
        }],
        "skins": [{"name": "bar_skin", "inverseBindMatrices": 4, "joints": [1, 2]}],
        "materials": [{
            "name": "bar_material",
            "pbrMetallicRoughness": {
                "baseColorFactor": [0.8, 0.3, 0.2, 1.0],
                "metallicFactor": 0.0,
                "roughnessFactor": 0.5,
            },
        }],
        "animations": [{
            "name": "bend",
            "samplers": [{"input": 5, "output": 6, "interpolation": "LINEAR"}],
            "channels": [{"sampler": 0, "target": {"node": 2, "path": "rotation"}}],
        }],
        "accessors": accessors,
        "bufferViews": views,
        "buffers": [{"byteLength": len(buffer_blob)}],
    }

    return gltf, buffer_blob


def write_gltf(gltf, buffer_blob, path):
    document = json.loads(json.dumps(gltf))
    document["buffers"][0]["uri"] = "data:application/octet-stream;base64," + \
        base64.b64encode(buffer_blob).decode("ascii")
    path.write_text(json.dumps(document, indent=2) + "\n")
    print(f"wrote {path.relative_to(ROOT)} ({path.stat().st_size} bytes)")


def write_glb(gltf, buffer_blob, path):
    json_blob = json.dumps(gltf, separators=(",", ":")).encode("utf-8")
    json_blob += b" " * (-len(json_blob) % 4)
    binary_blob = buffer_blob + b"\0" * (-len(buffer_blob) % 4)

    total = 12 + 8 + len(json_blob) + 8 + len(binary_blob)

    with path.open("wb") as handle:
        handle.write(struct.pack("<III", 0x46546C67, 2, total))
        handle.write(struct.pack("<II", len(json_blob), 0x4E4F534A))
        handle.write(json_blob)
        handle.write(struct.pack("<II", len(binary_blob), 0x004E4942))
        handle.write(binary_blob)

    print(f"wrote {path.relative_to(ROOT)} ({path.stat().st_size} bytes)")


def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    gltf, buffer_blob = build()
    write_gltf(gltf, buffer_blob, OUT_DIR / "skinned_bar.gltf")
    write_glb(gltf, buffer_blob, OUT_DIR / "skinned_bar.glb")


if __name__ == "__main__":
    main()
