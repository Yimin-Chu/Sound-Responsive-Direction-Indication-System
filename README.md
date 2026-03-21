Sound-Responsive Direction Indication System — Modular Embedded System with RTOS-Oriented Design

1.Designed and implemented a dual-microphone sound-responsive direction indication system on STM32 NUCLEO-F446ZE, with a modular architecture consisting of data acquisition, signal processing, display, and actuation pipelines, emphasizing real-time performance and future RTOS scalability.

2.Implemented continuous dual-channel audio sampling using ADC + DMA (circular/double-buffer mode) for MAX4466 microphones, reducing CPU overhead and enabling frame-based processing for parallel acquisition and computation.

3.Developed lightweight signal processing algorithms including dynamic baseline estimation, DC offset removal, sliding-window energy computation, and inter-channel differential comparison, enabling low-complexity direction estimation for near-field impulsive sound sources.

4.Designed adaptive noise gating, event-triggered thresholding, and anti-jitter/hysteresis logic to suppress false triggers caused by background noise, transient spikes, and channel fluctuations, improving robustness in noisy environments.

5.Controlled servo actuation via PWM-based angle mapping for physical direction indication, and implemented a real-time user interface using SSD1306 OLED over I2C to display signal intensity, threshold status, direction output, and system states, forming a complete sensing–decision–actuation loop.

6.Structured software using a producer–consumer model to decouple data acquisition and control logic, with clear module boundaries to support seamless migration to FreeRTOS-based multi-task scheduling (sampling, processing, display, and control tasks).
