import vec

struct surfinfo:
   x: int

struct line_seg3:
   start: xyz_f
   extent: xyz_f

class collision_shape:
   def move_to(p: xyz_f):
      pass()

private let po_shape = [collision_shape{}] :: collision_shape

struct physics_object:
   h: int

   def set_pos(p: xyz_f) -> physics_object:
      po_shape[h].move_to(p)
      return this

class c_capsule: collision_shape
   center_seg: line_seg3  
   radius: float

   def apply_overlap(ov: xyz_f):
      center_seg = line_seg3{center_seg.start + ov, center_seg.extent}

   def move_to(p: xyz_f):
      center_seg = line_seg3{p + xyz_z*radius, center_seg.extent}

   def set_height(h: float):
      let ext = normalize(center_seg.extent)
      center_seg = line_seg3{center_seg.start, ext*(max(h-2.0*radius, 0.0))}

class extruded_sphere: c_capsule

class csphere: collision_shape
   center: xyz_f 
   radius: float
   resolved_pushes = xyz_0

   def move_to(p: xyz_f):
      center = p + xyz_z*radius
      resolved_pushes = xyz_0

let shape = c_capsule{line_seg3{xyz_0, xyz_0}, 1.0}
let new_pos = xyz_0
let face_info = [] :: surfinfo
let phys_obj = physics_object{0}
shape.move_to(new_pos)
let contacts = [] 
let pos = new_pos
phys_obj.set_pos(pos)
