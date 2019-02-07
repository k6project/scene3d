bl_info = {
    "name": "Scene3D binary format", 
    "category": "Import-Export",
    "support": "TESTING"
}

import bpy

class S3DExport(bpy.types.Operator):
    bl_idname = "export_scene.s3d"
    bl_label  = "Export for Scene3D"
    def execute(self, context):
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
