import random
import string

def get_text():
    text = ''.join(random.choice(string.ascii_lowercase[0:9]) for i in range(random.randint(1,4)))
    return(text) 

def create_file(name):
    file_name = "./data/"+name+"/work.txt"
    with open(file_name, "w") as fd:
        for _ in range(100000):
            fd.write(get_text()+"\n")
            if(name == "initial"):
                fd.write(str(random.randint(1,1000))+ "\n")
        fd.write("--"+ "\n")
    fd.close()

create_file("find")
create_file("initial")
create_file("pref")
create_file("rem")
