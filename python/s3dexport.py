bl_info = {
    "name": "Scene3D binary format", 
    "category": "Import-Export",
    "support": "TESTING"
}

import bpy
import bmesh
import s3dconv
from bpy.props import StringProperty
from bpy_extras.io_utils import ExportHelper

class S3DExport(bpy.types.Operator, ExportHelper):
    bl_idname = "export_scene.s3d"
    bl_label  = "Export for Scene3D"
    bl_options = {'PRESET'}
    filename_ext = ".s3d"
    filter_glob = StringProperty(default="*.s3d", options={"HIDDEN"})
    def execute(self, context):
        s3dconv.begin(self.filepath)
        scene = context.scene
        for obj in scene.objects: 
            if obj.type == "MESH":
                mesh = obj.to_mesh(context.scene, True, settings='RENDER', calc_tessface=False)
                tmp = bmesh.new()
                tmp.from_mesh(mesh)
                bmesh.ops.triangulate(tmp, faces=tmp.faces)
                tmp.to_mesh(mesh)
                tmp.free()
                mesh.calc_normals_split()
                for face in mesh.polygons:
                    indices = []
                    for li in face.loop_indices:
                        loop = mesh.loops[li]
                        v = mesh.vertices[loop.vertex_index]
                        vdata = [ v.co.x, v.co.y, v.co.z ] + [loop.normal.x, loop.normal.y, loop.normal.z]
                        indices.append(s3dconv.add_vertex(vdata))
                    s3dconv.add_face(indices)
        s3dconv.end()
        return {"FINISHED"}

def menu_func_export(self, context):
    self.layout.operator("export_scene.s3d", text="Scene3D (.s3d)")

def register():
    bpy.utils.register_class(S3DExport)
    bpy.types.INFO_MT_file_export.append(menu_func_export)

def unregister():
    bpy.utils.unregister_class(S3DExport)
    bpy.types.INFO_MT_file_export.remove(menu_func_export)

if __name__ == "__main__":
    register()
