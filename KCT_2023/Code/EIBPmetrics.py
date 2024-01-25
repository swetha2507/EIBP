import re
import os

# initialize variables for calculating sum of eth_size, checking churn, and finding max conv_time
max_convergence_time = float('-inf')
sum_eth_size = 0
churn_count = 0
total_files = 0
min_down_time = float('+inf')

# define a regular expression to match the required lines
pattern = re.compile(r'^CONV_TIME:(\d+\.\d+)$')

# get the value for down_time from user input
down_time_pattern = re.compile(r'^IF_TIMER_DOWN:(\d+\.\d+)$')

for filename in os.listdir('.'):
    if filename.endswith('.txt'):
        # open the file and loop through each line
        with open(filename, 'r') as file:
            for line in file:
                # check if the line matches the pattern
                match = down_time_pattern.match(line.strip())
                if match:
                    # extract the values from the lines
                    down_time = float(match.group(1))
                    # update file_max_conv_time if necessary
                    if down_time < min_down_time:
                        min_down_time = down_time
                        

down_time = min_down_time

# loop through all files in the current directory
for filename in os.listdir('.'):
    if filename.endswith('.txt'):
        total_files += 1
        # open the file and loop through each line
        with open(filename, 'r') as file:
            # initialize variables for this file
            file_sum_eth_size = 0
            file_churn = False
            file_max_conv_time = float('-inf')
            for line in file:
                # check if the line matches the pattern
                match = pattern.match(line.strip())
                if match:
                    # get the next two lines that contain ETH_SIZE and CHURN_TRUE
                    eth_size_line = file.readline().strip()
                    churn_true_line = file.readline().strip()
                    # extract the values from the lines
                    conv_time = float(match.group(1))
                    eth_size = int(eth_size_line.split(':')[1])
                    file_churn = file_churn or (conv_time > down_time)
                    # update file_max_conv_time if necessary
                    if conv_time > file_max_conv_time and (conv_time-down_time)< 2:
                        file_max_conv_time = conv_time
                    # add eth_size to file_sum if conv_time is greater than down_time
                    if conv_time > down_time and (conv_time-down_time)< 2:
                        file_sum_eth_size += eth_size
            # update global variables with values for this file
            if file_max_conv_time > max_convergence_time:
                max_convergence_time = file_max_conv_time
            sum_eth_size += file_sum_eth_size
            if file_churn:
                churn_count += 1

# calculate percentage of files with churn true
if total_files > 0:
    churn_percent = (churn_count / total_files) * 100
else:
    churn_percent = 0

# calculate the difference between max_conv_time and down_time
convergence_time = max_convergence_time - down_time

# display the result
print(f"CONVERGENCE_TIME: {convergence_time} s")
print(f"OVERHEAD: {sum_eth_size} bytes")
print(f"CHURN PERCENTAGE: {churn_percent:.2f}%")
