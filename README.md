# shared_ptr_with_log

Combines `std::shared_ptr` with any log allowing to use different log objects for the same shared object.
One particular use case is when one shared "library" object is used in several "applications".
`shared_ptr_with_log` in combination with Boost.Log allows to direct log from different "applications"
into separate sinks/files. Log from shared "library" object will be directed into the right sink which
is difficult to achieve when using logging sources in a direct way. The cost is that all "library" and 
"application" api must use `shared_ptr_with_log` instead of objects directly.
