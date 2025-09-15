
target("glad")
    set_kind("static")
    add_files("src/gl.c")
    add_includedirs("include", { public = true, })
