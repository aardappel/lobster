import vec

   
def entry(bsp, orc, h):
   perform_model_collisions(bsp, orc, h)
   perform_trigger_collisions(bsp, orc, h)

class bsp_file:
   a: int

class orchestrator:
   a: int

//REPRODUCTION NOTE adding this as the type annotation to the body parameter in on_brush_collision
//     will make the assert go away.
def on_brush_collision_body(plane: int, pos: xyz_f, overlap: xyz_f) -> void

class collision_shape:
   def overlaps_aabb(minc: xyz_f, maxc: xyz_f) -> bool:
      return true

   def plane_split(plane_normal: xyz_f, plane_dist: float, body):
      pass()

   def on_brush_collision(bsp: bsp_file, brush: int, area_mins: xyz_f, area_maxs: xyz_f, body):
      pass()

   def bounding_sphere_radius() -> float:
      return 0.0
      
class csphere: collision_shape
   def on_brush_collision(bsp: bsp_file, brush: int, area_mins: xyz_f, area_maxs: xyz_f, body):
      body(4, xyz_1, xyz_0)


class brush_area:
   // bak_* constants
   kind: int
   mins: xyz_f
   maxs: xyz_f
   brushes: [int]
   model_ix: int

   is_trigger: bool

   meshes = [] :: [resource<mesh>]

   clusters = [] :: int

   def centroid() -> xyz_f:
      return mins + (maxs-mins)*0.5

   def contains(p):
      return (p >= mins) == xyz{true,true,true} and (p <= maxs) == xyz{true, true, true}

struct physics_object:
   h:int

struct shape_contact:
   face: int
   pos: xyz_f
   overlap: xyz_f  
   plane: int = -1

struct model_collision_info:
   obj: physics_object
   model: brush_area
   contact: shape_contact

struct trigger_collision_info:
   obj: physics_object
   trigger: brush_area

private let po_shape = [ csphere{} ] :: collision_shape
private let po_overlap = [ xyzw_0 ] :: xyzw_f
private let py_triggers = [] :: brush_area
private let py_models   = [] :: brush_area

def mag_squared(x):
   return x.x

private def for_brushes_colliding_with_shape(bsp: bsp_file, brush_areas: [brush_area], shape: collision_shape, body):
   for(brush_areas) area:
      if shape.overlaps_aabb(area.mins, area.maxs):
         for(area.brushes) brush_ix:
            shape.on_brush_collision(bsp, brush_ix, area.mins, area.maxs) plane_no, pos, ovlp:
               body(area, plane_no, pos, ovlp)

private def perform_model_collisions(bsp: bsp_file, orc: orchestrator, h: int):
   let reportables = []
   for_brushes_colliding_with_shape(bsp, py_models, po_shape[h]) model, plane_no, pos, ovlp:
      po_overlap[h] += xyzw(ovlp, 1.0)
      if mag_squared(ovlp) > 0.0:
         push(reportables, model_collision_info{physics_object{h}, model,
                                                shape_contact{-1, pos, ovlp, plane: plane_no}})

private def perform_trigger_collisions(bsp: bsp_file, orc: orchestrator, h: int):
   let reportables = []
   for_brushes_colliding_with_shape(bsp, py_triggers, po_shape[h]) trigger, pno, pos, ovlap:
      push(reportables,
           trigger_collision_info{physics_object{h}, trigger})


def on_collisions_with_model_brushes(bsp: bsp_file, shape: collision_shape, body):
   for_brushes_colliding_with_shape(bsp, py_models, shape, body)

def doit():
   let bsp = bsp_file{1}
   let orc = orchestrator{1}

   entry(bsp, orc, 0)
   on_collisions_with_model_brushes(bsp, collision_shape{}) area, plane, pos, ovlp:
      pass()

doit()
