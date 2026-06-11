**FreeRTOS based BareMetal Processor Monitor**
---
**Overview**

A preemptive FreeRTOS-based system running on STM32 that continuously monitors internal MCU parameters, logs system health information to external SPI flash, 
and provides an interactive command-line shell for runtime inspection.

The project demonstrates RTOS concepts such as task scheduling, inter-task communication, runtime statistics, memory monitoring, and persistent data logging.

---
**HARDWARE USED**
    STM32F446RE Nucleo

---
**Features**
System Monitoring:
-Processor Temperature Monitoring
-Internal Reference Voltage Monitoring (VREFINT)
-Heap Usage Monitoring
-Task Stack Usage Monitoring
-CPU Utilization Monitoring
-RTC Timekeeping

Persistent Logging:
-Timestamped system health records
-Storage in external W25Q32 SPI Flash
-Flash readback verification - ITM based (only for verification)

Interactive Shell:
Minimalistic shell that has options for seeing elaborate run-time stats, flash log data, rtc time configuration and chip erase.

---
**FreeRTOS Features Demonstrated**
-Preemptive Scheduling
-Task Priorities
-Task Notifications and synchronization
-Queues
-Message Buffers
-Runtime Statistics
-Stack Watermark Monitoring
-Heap Monitoring

---
**Architecture**




