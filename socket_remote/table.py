import glob
from statistics import mean

# Считываем данные из файлов serv_thread_*.txt
files_ts = sorted(glob.glob("serv_thread_*.txt"))
ts = []
for fname in files_ts:
    with open(fname) as f:
        lines = f.readlines()
        if lines:
            ts.extend([int(line.strip()) for line in lines if line.strip()])
print(ts)

# Считываем данные из файлов client_thread_*.txt
files_tc = sorted(glob.glob("client_thread_*.txt"))
tc = []
for fname in files_tc:
    with open(fname) as f:
        lines = f.readlines()
        if lines:
            tc.extend([int(line.strip()) for line in lines if line.strip()])
print(tc)

# Считываем данные из файлов epoll_server_times.txt
with open("epoll_server_times.txt") as f:
    es = f.readlines()
es = [int(es[_][:-1]) for _ in range(len(es))]
print(es)

# Считываем данные из файлов client_epoll_*.txt
files_ec = sorted(glob.glob("client_epoll_*.txt"))
ec = []
for fname in files_ec:
    with open(fname) as f:
        lines = f.readlines()
        if lines:
            ec.extend([int(line.strip()) for line in lines if line.strip()])
print(ec)

with open("times.csv", "w") as f:
    f.write("thread server,thread client,epoll server,epoll client\n")
    for i in range(min(len(ts), len(tc), len(es), len(ec))):
        f.write(f"{ts[i]},{tc[i]},{es[i]},{ec[i]}\n")
    f.write(f"{mean(ts)},{mean(tc)},{mean(es)},{mean(ec)}\n")
