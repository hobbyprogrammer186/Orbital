Quick Start: Orbital Programming Language
========================================

Features
--------

- Easy to use

Printing "Hello World"
----------------------

.. code-block:: orbital

    print "Hello World"


Getting Two Numbers from Users and Outputting Sum
-------------------------------------------------

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

File Read/Write
---------------

Read File Example:
.. code-block:: orbital
    filename = foo.txt # Replace The Example File Path With Existing File Path To View Content
    contents = read filename;
    print contents

Write File Example:
.. code-block:: orbital
    filename = foo.txt # Replace The Example File Path With File Path To Write.
    contents = input "Enter The File Contents:"
    write filename, contents

Notes
-----
- Write Function Can Be Overwrite The file when called in orbital interpreter. Please Be Careful.