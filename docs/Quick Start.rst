Quick Start: Orbital Programming Language
========================================

Features
--------

- Easy to use

Printing "Hello World"
----------------------

.. code-block:: orbital

    print "Hello World"


Getting Two Numbers from Users and Printing Their Sum
----------------------------------------------------

.. code-block:: orbital

    fn = input "Enter First Number:"
    sn = input "Enter Second Number:"
    print fn + sn


Defining and Calling Functions
------------------------------

.. code-block:: orbital

    SayHelloWorld:
        print "Hello World"

    SayHello(nickname):
        print "Hello " + nickname

    SayHelloWorld
    SayHello "Earth"


If / Else Statements
--------------------

.. code-block:: orbital

    x = input "Enter x:"
    y = input "Enter y:"

    if x <= 5:
        print "x <= 5"
    else:
        print "x > 5"

    if y >= 11:
        print "y >= 11"
    elif x > 8:
        print "x > 8"
    elif y == 10:
        print "y == 10"


Notes
-----

- Blocks are started using ``:`` (colon).
- There is no need for explicit block termination in functions or control statements.