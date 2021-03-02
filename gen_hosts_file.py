# Generate hosts.csv file
# Usage: python gen_hosts_file.py zedro.hosts /local/deploy/hosts.csv <l1-servers> <l1-per-server> <l1-cores> <l2-servers> <l2-per-server> <l2-cores> <l3-servers> <l3-per-server> <l3-cores> <kv-servers> <kv-per-server> <kv-cores> <clients> 

import sys

kv_base_port = 14000
l1_base_port = 10000
l2_base_port = 11000
l3_base_port = 13000

inp_hosts = sys.argv[1]
out_hosts = sys.argv[2]
num_l1_servers = int(sys.argv[3])
num_l1_per_server = int(sys.argv[4])
num_l1_cores = int(sys.argv[5])
num_l2_servers = int(sys.argv[6])
num_l2_per_server = int(sys.argv[7])
num_l2_cores = int(sys.argv[8])
num_l3_servers = int(sys.argv[9])
num_l3_per_server = int(sys.argv[10])
num_l3_cores = int(sys.argv[11])
num_kv_servers = int(sys.argv[12])
num_kv_per_server = int(sys.argv[13])
num_kv_cores = int(sys.argv[14])
num_clients = int(sys.argv[15])

cores_per_server = 32
cores = [str(i) for i in range(cores_per_server)]

servers = []
f = open(inp_hosts, 'r')
for line in f:
    if line.strip() == '':
        continue
    servers.append(line.strip())
f.close()

print('%d servers' % (len(servers)))

required_servers = num_l1_servers + num_l2_servers + num_l3_servers + num_kv_servers + num_clients
if required_servers > len(servers):
    print('Not enough servers')

f = open(out_hosts, 'w')
idx = 0

idx += num_clients
for i in range(num_kv_servers):
    core_idx = 0
    for j in range(num_kv_per_server):
        f.write('kv_%d_%d KV %s %d %s\n' % (i, j, servers[idx], kv_base_port + j, ','.join(cores[core_idx:core_idx+num_kv_cores])))
        core_idx += num_kv_cores
    idx += 1

for i in range(num_l1_servers):
    core_idx = 0
    for j in range(num_l1_per_server):
        f.write('l1_%d_%d L1 %s %d %s\n' % (i, j, servers[idx], l1_base_port + j, ','.join(cores[core_idx:core_idx+num_l1_cores])))
        core_idx += num_l1_cores
    idx += 1

for i in range(num_l2_servers):
    core_idx = 0
    for j in range(num_l2_per_server):
        f.write('l2_%d_%d L2 %s %d %s\n' % (i, j, servers[idx], l2_base_port + j, ','.join(cores[core_idx:core_idx+num_l2_cores])))
        core_idx += num_l2_cores
    idx += 1

for i in range(num_l3_servers):
    core_idx = 0
    for j in range(num_l3_per_server):
        f.write('l3_%d_%d L3 %s %d %s\n' % (i, j, servers[idx], l3_base_port + j, ','.join(cores[core_idx:core_idx+num_l3_cores])))
        core_idx += num_l3_cores
    idx += 1

f.close()




