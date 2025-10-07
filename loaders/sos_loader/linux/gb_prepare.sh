#!/bin/bash

# // SPDX-License-Identifier: GPL-2.0-only OR Proprietary
# // Author: Miao Yu (Carnegie Mellon University)
# // Author: Virgil Gligor (Carnegie Mellon University)

# sudo ./gb_prepare.sh <CPU_idx for SecOS> 

# Function to exclude a specific CPU from IRQs
exclude_irq_from_cpu() {
    exclude_cpu=$1  # CPU to exclude from IRQ handling

    # Get the number of CPUs
    cpu_count=$(nproc)

    # Generate CPU list excluding the specified CPU
    cpu_list=""
    for i in $(seq 0 $(($cpu_count - 1))); do
        if [ "$i" -ne "$exclude_cpu" ]; then
            if [ -z "$cpu_list" ]; then
                cpu_list="$i"
            else
                cpu_list="$cpu_list,$i"
            fi
        fi
    done

    echo "Excluding CPU $exclude_cpu from IRQ handling. Using CPUs: $cpu_list"

    # Iterate over each IRQ and set the affinity excluding the specified CPU
    for irq_dir in /proc/irq/*; do
        if [ -d "$irq_dir" ]; then
            irq_num=$(basename "$irq_dir")
            echo "Affining IRQ $irq_num to CPUs $cpu_list"
            echo $cpu_list > "$irq_dir/smp_affinity_list"
        fi
    done
}

secagent_reload() {
    has_module=`lsmod | grep secagent`

    if test -n "$has_module"
    then
        rmmod secagent
    fi

    insmod ./secagent.ko
}



# Flush file cache first
sync; echo 3 > /proc/sys/vm/drop_caches

# Disable IRQ balancer
sudo service irqbalance stop

# Disable RCU cpu stall detector
sudo echo 1 > /sys/module/rcutree/parameters/rcu_cpu_stall_suppress

# Remove IRQs from the given CPU
exclude_irq_from_cpu $1

# Load SecAgent, reload it if exists
secagent_reload