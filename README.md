# ESP-IDF Binary Semaphore with PWM and UART Communication

## Overview

This project demonstrates the use of FreeRTOS Binary Semaphores in ESP-IDF for task synchronization. The application uses UART communication to receive data and controls PWM output based on task interactions and received commands.

## Features

* FreeRTOS Binary Semaphore synchronization
* UART communication using ESP-IDF UART driver
* PWM signal generation using LEDC driver
* Multiple task management using FreeRTOS
* Event-driven task synchronization
* ESP32-based embedded application

## Hardware Requirements

* ESP32 Development Board
* USB Cable
* PC with ESP-IDF installed

## Software Requirements

* ESP-IDF
* FreeRTOS
* CMake
* Python (required by ESP-IDF)

## Project Structure

```text
.
├── main/
│   ├── main.c
│   ├── uart_driver.c
│   ├── uart_driver.h
│   ├── pwm_driver.c
│   └── pwm_driver.h
├── CMakeLists.txt
├── sdkconfig.defaults
└── README.md
```

## Working Principle

1. UART Task waits for incoming data.
2. When valid data is received, a Binary Semaphore is given.
3. PWM Control Task waits for the Binary Semaphore.
4. After receiving the semaphore, the PWM duty cycle is updated.
5. The tasks continue running and synchronize through FreeRTOS mechanisms.

## FreeRTOS Concepts Used

### Binary Semaphore

Binary Semaphores are used for task synchronization where one task signals another task to perform an action.

Functions used:

```c
xSemaphoreCreateBinary();
xSemaphoreTake();
xSemaphoreGive();
```

### Task Management

Functions used:

```c
xTaskCreate();
vTaskDelay();
```

## UART Configuration

* Configurable baud rate
* UART receive handling
* Command-based communication

## PWM Configuration

* LEDC PWM driver
* Adjustable duty cycle
* Configurable frequency

## Build and Flash

Configure target:

```bash
idf.py set-target esp32
```

Build project:

```bash
idf.py build
```

Flash firmware:

```bash
idf.py flash
```

Monitor serial output:

```bash
idf.py monitor
```

## Learning Objectives

This project helps understand:

* FreeRTOS task synchronization
* Binary Semaphore usage
* UART communication in ESP-IDF
* PWM generation using LEDC
* Embedded firmware architecture
* Real-time application development

## Author

Dhruv Dave

## License

This project is provided for learning and educational purposes.

