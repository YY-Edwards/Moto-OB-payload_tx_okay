[2016-03-15 15:55]
1、FreeRtos移植到“倒地报警”Demo工程；
2、将Demo中的中断（my_INTC）配置方式改为默认（intc.c）方式，以相应FreeRtos中Port.c的vTick中断服务程序
3、整理代码，将不同接口（功能）从main.c中剥离

