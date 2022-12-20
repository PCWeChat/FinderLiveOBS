import os
import subprocess

# protoc --proto_path=./proto --cpp_out=./../flproto/proto ./proto/xxxx.proto
gen_all_proto_file = True
for root, dirs, files in os.walk('./proto'):
    if gen_all_proto_file:
        for name in files:
            if name.endswith('.proto'):
                proto_file_path = [os.path.join(root, name)]
                if subprocess.call(['protoc', '--proto_path=./proto', '--cpp_out=./../flproto/proto'] + proto_file_path) != 0:
                    gen_all_proto_file = False
                    print(proto_file_path)

if gen_all_proto_file:
    print('protoc gen all proto success')
