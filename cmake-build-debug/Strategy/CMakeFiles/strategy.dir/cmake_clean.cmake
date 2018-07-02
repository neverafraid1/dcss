file(REMOVE_RECURSE
  "strategy.pdb"
  "strategy"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/strategy.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
