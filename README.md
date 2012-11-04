sensor.im
=========

Embedded Zabbix Agent Sensor

I created a stand alone, hardware ZABBIX Agent (running Zabbix Agent-like firmware directly on micro-controller) using Arduino Mega 2560 Board + Ethernet Shield (Wiznet W5100). This device equipped with DHT22 temperature-humidity sensor. 

Current (ZSA1-E model) Features:

1. Measures temperature and humidity using calibrated digital sensor
2. Direct monitoring with Zabbix Server (behave like Zabbix agent in passive mode)
3. Can support PoE 802.3 (need special Ethernet Shield with PoE support)
4. Cheap: 50~60$ for all parts price (based on Alibaba and Sparkfun prices, shipping included)
5. Can be easy assembled (minimal soldering and enclosure modification)
6. Web based network setting setup (save network settings in EEPROM) 