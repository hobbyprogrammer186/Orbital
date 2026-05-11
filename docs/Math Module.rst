OMath Module: Mathematical Functions
=====================================

Usage Examples
-------------

**Calculating Square Root**

.. code-block:: orbital

    x = 16
    result = sqrt x
    print result  # Output: 4

**Using Power Function**

.. code-block:: orbital

    base = 2
    exponent = 8
    result = pow base, exponent
    print result  # Output: 256

**Trigonometry**

.. code-block:: orbital

    angle = 3.14159  # Approximately pi
    sine_val = sin angle
    cosine_val = cos angle
    print sine_val
    print cosine_val

**Logarithms**

.. code-block:: orbital

    num = 100
    natural_log = log1 num
    base10_log = log10 num
    print natural_log
    print base10_log

**Rounding Numbers**

.. code-block:: orbital

    x = 3.7
    y = 3.2
    print celi x    # Output: 4
    print floor y   # Output: 3
    print round x   # Output: 4

Notes
-----

- All trigonometric functions expect and return angles in radians
- The functions return ``variant<double, long, int, std::string>`` to handle different numeric types
- If an unknown function is called, the module returns ``0L``