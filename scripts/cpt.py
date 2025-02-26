#!/usr/bin/python3

import json
import sys
import struct

class CPT:
    def  __init__(self):  
        simple_types = [ "int", "unsigned int", "unsigned long", "unsigned long long"]
        # create hierarchical name and convert position to absolute position
        self.name2pos = {}
        self.name2hash = {}
        self.hash2type = {}
        self.next_seg_num = 0
        self.seg_pos = 0

    def seg_info(self, rec):
        seg_name = rec['seg_name']
        seg_num = int(rec['seg_num'])
        seg_size = int(rec['seg_size'])
        print(f"Processing segment {seg_num} {seg_name} starting at {self.seg_pos} size {seg_size}")
        if (seg_num != self.next_seg_num):
            print(f"seg_num mismatch {self.next_seg_num}", file=sys.stderr)
            exit(1)
        else:
            self.next_seg_num = self.next_seg_num + 1
        names = rec['names']
        for n in names:
            long_name = f"{seg_name}.{n['name']}"
            pos = self.seg_pos + int(n['pos'])
            print(f"\t{n['name']} {n['pos']} -> {long_name} {pos}")
            # save the name, the position, and the type hash
            self.name2pos[long_name] = pos
            self.name2hash[long_name] = int(n['hash_code'], base=16)
        # update base position
        self.seg_pos = self.seg_pos + int(rec['seg_size'])
        #print(g_name2pos)

    def type_info(self, rec):
        self.hash2type
        for t in rec['type_info']:
            hash_code = int(t['hash_code'],base=16)
            name = t['name']
            # size = int(t['size'])
            self.hash2type[hash_code] = name

    # TODO combine into tuple
    def getPosition(self, name):
        return self.name2pos[name];

    def getType(self, name):
        return self.hash2type[self.name2hash[name]]
        
    def load(self, json_file, cpt_file):
        self.hash2type
        self.name2pos
        with open(json_file) as schema_file:
            schema = json.load(schema_file);

        chkpt = schema['checkpoint_def']
        # seg_count = 0
        for rec in chkpt:
            if rec['rec_type'] == "seg_info":
                self.seg_info(rec)
            if rec['rec_type'] == "type_info":
                self.type_info(rec)

        print("# Listing variable name, position, and type\n")
        for n in self.name2pos:
            pos = self.name2pos[n]
            hash = self.name2hash[n]
            typestring = self.hash2type[hash]
            print(f"{n} {pos} {typestring}")

        print(f"\nLoading checkpoint file {cpt_file}")
        with open(cpt_file, mode='rb') as datafile:
            self.blob = datafile.read()
        
        # set look-up list and expected data
        obj_list = []
        expected = {}

        obj_list.extend(['loaded_libraries.seg1begin']);
        expected['loaded_libraries.seg1begin'] = 0xa5a5a5a5a5a5bb01;
        obj_list.extend(['loaded_libraries.seg1end']);
        expected['loaded_libraries.seg1end'] = 0xa5a5a5a5a5a5ee01;

        obj_list.extend(['simulation_impl.seg2begin']);
        expected['simulation_impl.seg2begin'] = 0xa5a5a5a5a5a5bb02;
        obj_list.extend(['simulation_impl.seg2end']);
        expected['simulation_impl.seg2end'] = 0xa5a5a5a5a5a5ee02;
        
        for s in obj_list:
            pos = self.name2pos[s]
            type = self.hash2type[self.name2hash[s]]
            value = struct.unpack_from("Q", self.blob, pos)
            print(f"{s} type='{type}' pos=0x{pos:x}  value=0x{value[0]:x}")
            if expected[s] != value[0]:
                print(f"ERROR: mismatch E=0x{expected[s]:x} A=0x{value[0]:x}", file=sys.stderr)
                exit(2)

if __name__ == "__main__":
    print("cpt.py v0.0")
    
