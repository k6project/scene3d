function initWebGL2(canvasId)
{
    var canvas = document.getElementById(canvasId);
    if (!canvas) window.alert("No canvas object!!!");
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    var gl = canvas.getContext("webgl2");
    if (!gl) window.alert("No WebGL2 context!!!");
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;
    gl.clearColor(0, 0.7, 1, 1);
    return gl;
}

function createShader(gl, vsId, fsId)
{
    var vs = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vs, document.getElementById(vsId).textContent);
    gl.compileShader(vs);
    if (!gl.getShaderParameter(vs, gl.COMPILE_STATUS)) 
    {
        window.alert(gl.getShaderInfoLog(vs));
        gl.deleteShader(vs);
        return null;
    }
    var fs = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fs, document.getElementById(fsId).textContent);
    gl.compileShader(fs);
    if (!gl.getShaderParameter(fs, gl.COMPILE_STATUS)) 
    {
        window.alert(gl.getShaderInfoLog(fs));
        gl.deleteShader(vs);
        gl.deleteShader(fs);
        return null;
    }
    shader = gl.createProgram();
    gl.attachShader(shader, vs);
    gl.attachShader(shader, fs);
    gl.linkProgram(shader);
    if (!gl.getProgramParameter(shader, gl.LINK_STATUS))
    {
        window.alert(gl.getProgramInfoLog(shader));
        gl.deleteProgram(shader);
        shader = null;
    }
    gl.deleteShader(vs);
    gl.deleteShader(fs);
    return shader;
}
