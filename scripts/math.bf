

+++++  # set cell 0 to 5


# copy cell 0 to 1
[
    -
    >+>+
    <<
]
>>[-<<+>>]<<

# multiply cell 0 by cell 1, and store in cell 2
[-
    >
    [->+>+<<] # increment cells 2 and 3
    <
    
    >>>
    [-<<+>>] # restore value of cell 1 from saved copy at cell 3
    <<<
]

# multiply cell 2 by 2 and store in cell 1
>>
[-
    <++>
]

<.
