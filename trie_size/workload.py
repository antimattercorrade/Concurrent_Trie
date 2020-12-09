import random
import string

TESTS = 100

def get_text(val):
    text = ''.join(random.choice(string.ascii_lowercase[0:9]) for i in range(random.randint(1,val)))
    return(text)

def create_file(name):
    for i in range(TESTS):
        file_name = "./data/"+name+"/"+str(i+1)+".txt"
        with open(file_name, "w") as fd:
            for _ in range(15000):
                fd.write(get_text(i+1)+"\n")
                if(name == "initial"):
                    fd.write(str(random.randint(1,1000))+ "\n")
            fd.write("--"+ "\n")
        fd.close()

create_file("find")
create_file("initial")
create_file("pref")
create_file("rem")
