#!/usr/bin/python3

import cpt
import struct
import sys

if __name__ == "__main__":
    # This script specifically checks checkpoint results generated by schema-save.sh.
    # There are 4 components, 4 threads,  and 10 checkpoints generated.
    cpt_pfx_list=[
        "2d_SAVE_/2d_SAVE__0_1000000/2d_SAVE__0_1000000",
        "2d_SAVE_/2d_SAVE__1_2000000/2d_SAVE__1_2000000",
        "2d_SAVE_/2d_SAVE__2_3000000/2d_SAVE__2_3000000",
        "2d_SAVE_/2d_SAVE__3_4000000/2d_SAVE__3_4000000",
        "2d_SAVE_/2d_SAVE__4_5000000/2d_SAVE__4_5000000",
        "2d_SAVE_/2d_SAVE__5_6000000/2d_SAVE__5_6000000",
        "2d_SAVE_/2d_SAVE__6_7000000/2d_SAVE__6_7000000",
        "2d_SAVE_/2d_SAVE__7_8000000/2d_SAVE__7_8000000",
        "2d_SAVE_/2d_SAVE__8_9000000/2d_SAVE__8_9000000",
        "2d_SAVE_/2d_SAVE__9_10000000/2d_SAVE__9_10000000",
        "2d_SAVE_/2d_SAVE__10_11000000/2d_SAVE__10_11000000",
    ]

    for pfx in cpt_pfx_list:
        for rank_thread in ["_0_0", "_0_1", "_0_2", "_0_3"]:
            fn=f"{pfx}{rank_thread}"
            print(f"Loading {fn}.bin using {fn}.schema.json")
            # Create checkpoint object from the checkpoint files
            cptObj = cpt.CPT()
            # Load and check the framing of non-component specific segments
            cptObj.load(f"{fn}.schema.json", f"{fn}.bin")
            # Check our custom component markers
            obj_list = []
            expected = {}
            if rank_thread=="_0_0":
                obj_list.extend(['cp_0_0.segcbegin']);
                expected['cp_0_0.segcbegin'] = 0xa5a5a5a5a5a5bb0c;
                obj_list.extend(['cp_0_0.segcend'])
                expected['cp_0_0.segcend'] = 0xa5a5a5a5a5a5ee0c
            elif rank_thread=="_0_1":
                obj_list.extend(['cp_0_1.cptBegin'])
                expected['cp_0_1.cptBegin'] = 0xffb000000001b1ff
                obj_list.extend(['cp_0_1.cptEnd'])
                expected['cp_0_1.cptEnd'] = 0xffe000000001e1ff
            elif rank_thread=="_0_2":
                obj_list.extend(['cp_1_0.cptBegin'])
                expected['cp_1_0.cptBegin'] = 0xffb000000002b1ff
                obj_list.extend(['cp_1_0.cptEnd'])
                expected['cp_1_0.cptEnd'] = 0xffe000000002e1ff
            elif rank_thread=="_0_3":
                obj_list.extend(['cp_1_1.cptBegin'])
                expected['cp_1_1.cptBegin'] = 0xffb000000003b1ff
                obj_list.extend(['cp_1_1.cptEnd'])
                expected['cp_1_1.cptEnd'] = 0xffe000000003e1ff
    
            for s in obj_list:
                print(f"Looking for {s} in {fn}")
                pos = cptObj.getPosition(s)
                type = cptObj.getType(s)
                value = cptObj.getValue(s)
                print(f"{s} type='{type}' pos=0x{pos:x}  value=0x{value[0]:x}")
                if expected[s] != value[0]:
                    print(f"ERROR: mismatch E=0x{expected[s]:x} A=0x{value[0]:x}", file=sys.stderr)
                    exit(2)