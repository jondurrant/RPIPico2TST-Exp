add_executable(${NAME}
        main.cpp
        Agent.cpp
    	Counter.cpp
		Worker.cpp
		TSTAgent.cpp
		TSTMetrics.cpp
        )

# Pull in our pico_stdlib which pulls in commonly used features
target_link_libraries(${NAME} 
	pico_stdlib
	pi_spigot
	FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
	freertos_config #FREERTOS_PORT
	tst
	)

# enable usb output, disable uart output
pico_enable_stdio_usb(${NAME} 1)
pico_enable_stdio_uart(${NAME} 0)


# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})