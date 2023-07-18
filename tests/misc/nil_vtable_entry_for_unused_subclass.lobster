import vec

struct surfinfo:
   x: int

struct line_seg3:
   start: float3
   extent: float3

class collision_shape:
   def move_to(p: float3):
      pass()

private let po_shape = [collision_shape{}] :: collision_shape

struct physics_object:
   h: int

   def set_pos(p: float3) -> physics_object:
      po_shape[h].move_to(p)
      return this

class c_capsule: collision_shape
   center_seg: line_seg3  
   radius: float

   def apply_overlap(ov: float3):
      center_seg = line_seg3{center_seg.start + ov, center_seg.extent}

   def move_to(p: float3):
      center_seg = line_seg3{p + float3_z*radius, center_seg.extent}

   def set_height(h: float):
      let ext = normalize(center_seg.extent)
      center_seg = line_seg3{center_seg.start, ext*(max(h-2.0*radius, 0.0))}

class extruded_sphere: c_capsule

class csphere: collision_shape
   center: float3 
   radius: float
   resolved_pushes = float3_0

   def move_to(p: float3):
      center = p + float3_z*radius
      resolved_pushes = float3_0

let shape = c_capsule{line_seg3{float3_0, float3_0}, 1.0}
let new_pos = float3_0
let face_info = [] :: surfinfo
let phys_obj = physics_object{0}
shape.move_to(new_pos)
let contacts = [] 
let pos = new_pos
phys_obj.set_pos(pos)
