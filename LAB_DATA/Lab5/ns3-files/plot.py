import matplotlib.pyplot as plt




THPR = []
MM = []


for i in ['1','5','10','20','30']:
    print("____________________________________________")
    print("NUM_STATIONS :== " + i)
    f = open(i + "stats.txt")
    start = 1
    counter = 0
    THRPT_LIST = []
    MM_LIST = []
    for l in f:
        if start == 1:
            start = 0
            continue
        if start == 0:
            D = l.split()
            THRPT_LIST.append(float(D[3]))
            MM_LIST.append(float(D[1])/float(D[2]))
            counter += 1

    thpr= sum(THRPT_LIST)/counter
    mm = sum(MM_LIST)/counter

    THPR.append(thpr)
    MM.append(mm)

    print("steps = ", counter)
    print("Avg Throighput : ", thpr )
    print("Avg Min-Max : ", mm )






print("=======================================================") 
print("=======================================================") 


print(THPR)
print(MM)











plt.plot([1,5,10,20,30],THPR,marker = "o",color = 'r')
plt.ylabel("Average Total Throughput")
plt.xlabel("No: of Clients")
plt.title("ROLL : 200050095")
plt.show()
plt.savefig("Throughput.png")




plt.plot([1,5,10,20,30],MM,marker = "o",color = 'r')
plt.ylabel("Avg MIN-to-MAX ratio")
plt.xlabel("No: of Clients")
plt.title("ROLL : 200050095")
plt.show()
plt.savefig("Min_Max.png")
