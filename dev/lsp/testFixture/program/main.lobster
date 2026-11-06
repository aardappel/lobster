// A program whose helper uses what the main file declares before importing it.
let shared = 1
import .sub.helper
print helper()
