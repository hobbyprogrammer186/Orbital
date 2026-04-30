add_library(orbtlio
	orbtlio.cpp
)

set_target_properties(orbtlio PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)