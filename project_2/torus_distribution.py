torus_size = 10
nb_workers = 2

out = ""
for i in range(0, torus_size):
    for j in range(0, torus_size):
        machine = (i + j * torus_size) % nb_workers 
        out += "{}".format(machine + 1)
    out += "\n"
print(out)
