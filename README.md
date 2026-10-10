BLUETOOTH-BASED-WIRELESS-RESTUARANT-ORDERING-SYSTEM:

This project uses the ARM7 LPC2129, keypad, LCD, and HC-05 Bluetooth module to manage food orders and status updates. An LED indicates order status, reducing communication errors and improving restaurant efficiency.


Project Overview

The Bluetooth-Based Wireless Restaurant Ordering System is an embedded system designed to simplify restaurant food ordering and order-status monitoring. It uses the ARM7 LPC2129 microcontroller, a keypad, an LCD, and an HC-05 Bluetooth module to improve communication between restaurant staff and customers.

Objectives

- Simplify the restaurant ordering process.
- Reduce manual communication and ordering errors.
- Display menu and order details on an LCD.
- Send order information wirelessly using Bluetooth.
- Provide order-status updates such as READY and DISPATCHED.

Hardware Requirements

- ARM7 LPC2129 Microcontroller
- 4×4 Matrix Keypad
- 16×2 LCD Display
- HC-05 Bluetooth Module
- LED for Order Status Indication
- Power Supply
- Connecting Wires

Software Requirements
- Embedded C
- Keil µVision IDE
- ARM7-compatible compiler
- Flash Magic or a compatible programming tool
- Bluetooth terminal application for mobile communication

Working Principle
1. The user enters the table number using the keypad.
2. Food items are selected through the keypad.
3. The LCD displays the menu and order details.
4. The HC-05 Bluetooth module transmits order information to a paired mobile device.
5. Order-status commands, such as READY and DISPATCHED, are received through Bluetooth.
6. The LED indicates the order status according to the program logic.

 System Workflow
**Keypad Input → LPC2129 Processing → LCD Display → Bluetooth Communication → Order Status Update**


 Implementation Steps
1. Connect the keypad, LCD, HC-05 module, and LED to the LPC2129.
2. Open `project.c` in Keil µVision.
3. Configure the target microcontroller and required project settings.
4. Compile the program and resolve any errors.
5. Flash the generated firmware onto the LPC2129.
6. Pair a mobile device with the HC-05 Bluetooth module.
7. Test food selection, order transmission, and status updates.

Advantages
- Reduces manual ordering errors.
- Improves communication efficiency.
- Enables wireless order transmission.
- Provides simple order-status indication.
- Offers a cost-effective embedded solution for restaurants.

Future Enhancements
- Develop a dedicated mobile application.
- Add digital billing and payment support.
- Maintain order history.
- Support multiple tables and simultaneous orders.
- Integrate a kitchen display system.

 Applications
- Restaurants and cafes
- Hotels and food courts
- Small food-service businesses
- Educational embedded-systems projects

 Technologies Used
- Microcontroller: ARM7 LPC2129
- Programming Language: Embedded C
- Display: 16×2 LCD
- Input: 4×4 Keypad
- Communication:HC-05 Bluetooth
- Status Indicator: LED

BLOCK DIAGRAM:
