=UrbiDuino=
[http://code.google.com/p/urbiduino/ http://urbiduino.googlecode.com/svn/trunk/images/UrbiDuino.png]

Goal of this project is implementation URBI for Arduino (http://www.urbiforge.org/index.php/UrbiOpenSourceContestProjects/Noonv)

== URBI and Arduino ==

*URBI* (Universal Real-time Behavior Interface) is an open source cross-platform software platform in C++ used to develop applications for robotics and complex systems. URBI is based on the UObject distributed *C++* component architecture. 
URBI includes the urbiScript orchestration language which is a parallel and event-driven script language. UObject components can be plugged into urbiscript and appear as native objects that can be scripted to specify their interactions and data exchanges.<br>
[http://en.wikipedia.org/wiki/URBI]<br>
[http://gostai.com]<br>
[http://www.urbiforge.com]<br>
[http://robocraft.ru/blog/algorithm/288.html] (rus)

*Arduino* is a single-board microcontroller and a software suite for programming it. The hardware consists of a simple open hardware design for the controller with an Atmel AVR processor (mostly ATMega168 or ATMega328) and on-board I/O  support. The software consists of a standard programming language and the boot loader that runs on the board.<br>
[http://en.wikipedia.org/wiki/Arduino]<br>
[http://arduino.cc]<br>
[http://robocraft.ru/blog/arduino/14.html] (rus)<br>


== UrbiDuino implementation ==
Arduinos are 8bit microcontrolers with no more than a few kilobytes of RAM (ATMega168 - 1Kb RAM). <br>
URBI is too big to run on it. So URBI is not embeddable in the Arduino.
<br>
However, we could use Arduino boards controlled by urbiScript on URBI-server running on a PC connected to the Arduino via a USB/Serial interface (or wireless technology: XBee, Bluetooth).<br>
[http://robocraft.ru/blog/algorithm/290.html http://urbiduino.googlecode.com/svn/trunk/images/pc-usb-arduino-robot.png]


<wiki:video url="http://www.youtube.com/watch?v=w4fnR9boiio"/>


== Usage ==
upload Arduino.dll and Arduino.u
run Arduino.u
{{{
=>var a = Arduino.new(19, 57600); // 19 - USB Serial of Arduino, 57600 - rate of StandardFirmata
#[0004240062] object_561
=> a.pinMode(13, OUTPUT); // set 13 pin led L as OUTPUT
=> a.digitalWrite(13,HIGH); // set pin HIGH
=> a.digitalWrite(13,LOW);
}}}

[http://code.google.com/p/urbiduino/ http://urbiduino.googlecode.com/svn/trunk/images/urbi-console.png]
 
== Lets URBI it! ==

Third place in Urbi Contest 2010! :)

[http://www.urbiforge.org/index.php/Main/UrbiContest2010 http://www.gostai.com/static/img/urbiforge/contest.jpg]
