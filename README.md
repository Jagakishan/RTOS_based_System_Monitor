**FreeRTOS based BareMetal Processor Monitor**
---
**Overview**

A preemptive FreeRTOS-based system running on STM32 that continuously monitors internal MCU parameters, logs system health information to external SPI flash, 
and provides an interactive command-line shell for runtime inspection.

Apart from the flash driver, every other driver involved is BareMetal implementation from initialization to usage. The falsh driver ws inherited from an older project, so it uses HAL functions.

---
**HARDWARE USED**

    STM32F446RE Nucleo

---
**Features**

**System Monitoring:**

-Processor Temperature Monitoring

-Internal Reference Voltage Monitoring (VREFINT)

-Heap Usage Monitoring

-Task Stack Usage Monitoring (Not implemented in final project as it involves task usage of each task, task number increased, so had to cut it off)

-CPU Utilization Monitoring (Based on time of idle hook run time)

-RTC Timekeeping

--

**Persistent Logging:**

-Timestamped system health records

-Storage in external W25Q32 SPI Flash

-Flash readback verification - ITM based (only for verification)



**Interactive Shell:**

Minimalistic shell that has options for seeing elaborate run-time stats, flash log data, rtc time configuration and chip erase.


**FreeRTOS Features Demonstrated**

-Preemptive Scheduling

-Task Priorities 

-Direct Task Notifications and synchronization

-Queues

-Message Buffers

-Runtime Statistics

-Stack Watermark Monitoring

-Heap Monitoring

---
**Architecture**

<img width="1006" height="688" alt="image" src="https://github.com/user-attachments/assets/162b4585-3c47-4254-b2ed-129e738a37f6" />


---
**Author**

Jagakishan S.K

---



