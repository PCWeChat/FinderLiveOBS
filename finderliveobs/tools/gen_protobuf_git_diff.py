import os
import subprocess
# --> pip3 install gitpython
import git

cwd_path = os.getcwd()
git_path = '/'.join(cwd_path.split('\\')[:-2])
repo = git.Repo(git_path)

modified_files = [item.a_path for item in repo.index.diff(None)]
staged_files = [item.a_path for item in repo.index.diff('HEAD')]
check_file_list = staged_files + modified_files + repo.untracked_files

gen_git_diff_proto_file_count = 0
gen_git_diff_proto_file = True

for file_path in check_file_list:
    if file_path.endswith(".proto"):
        gen_git_diff_proto_file_count += 1
        proto_file_path = ['./proto/' + (file_path.split('/')[-1])]
        if subprocess.call(['protoc', '--proto_path=./proto', '--cpp_out=./../flproto/proto'] + proto_file_path) != 0:
            gen_git_diff_proto_file = False
            print(proto_file_path)

if gen_git_diff_proto_file and gen_git_diff_proto_file_count != 0:
    print('protoc gen diff proto success')
