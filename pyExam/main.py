import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("D:/Jonas/AAU/P8/Exam/graphs/fig1C.csv")
agents = df['agentname'].unique()

for agent in agents:
    agent_data = df[df['agentname'] == agent]
    plt.plot(agent_data['time'], agent_data['agentamount'], label=agent)

plt.xlabel('Time')
plt.ylabel('Amount')
plt.title('Data set')
plt.legend()
plt.show()
