#version 410

in vec4 color;
out vec4 gl_FragColor;

in vec2 uvf;
in vec3 normalf;
in vec3 vertexf;

uniform sampler2D _MainTex;

const vec3 lightPos = vec3(1.0,1.0,1.0);
const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);

void main() {
//gl_FragColor = texture(_MainTex, uvf);

  vec3 normal = normalize(normalf); 
  vec3 lightDir = normalize(lightPos - vertexf);

  float lambertian = max(dot(lightDir, normal), 0.0);
  float specular = 0.0;

  if(lambertian > 0.0) {

    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 viewDir = normalize(-vertexf);

    float specAngle = max(dot(reflectDir, viewDir), 0.0);
    specular = pow(specAngle, 4.0);

    specular = pow(specAngle, 16.0);

  }

  gl_FragColor = texture(_MainTex, uvf) + vec4( lambertian*diffuseColor + specular*specColor, 1.0);
  
  }