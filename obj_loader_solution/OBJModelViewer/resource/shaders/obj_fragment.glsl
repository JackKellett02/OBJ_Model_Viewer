#version 400

smooth in vec4 vertPos;
smooth in vec4 vertNormal;
smooth in vec2 vertUV;

out vec4 outputColour;

uniform float lightStrength;

uniform vec4 camPos;

uniform vec4 kA;
uniform vec4 kD;
uniform vec4 kS;

//Uniform for whether or not textures are used.
uniform int textureUsed;

//Uniforms for texture data.
uniform sampler2D DiffuseTexture;
uniform sampler2D SpecularTexture;
uniform sampler2D NormalTexture;

vec3 iA = vec3(0.25f, 0.25f, 0.25f);
vec3 iD = vec3(1.0f, 1.0f, 1.0f);
vec3 iS = vec3(1.0f, 1.0f, 1.0f);

vec4 lightDir = normalize(vec4(0.0f) - vec4(10.0f, 8.0f, 1.0f, 0.0f));

//TODO:: FIGURE OUT HOW TO MAKE THE TEXTURE DATA HIGHER WEIGHTED.
void main()
{
	//Calculate Correct Normal Value From passed in value.
	float nDl = max(0.0f, dot(normalize(vertNormal), -lightDir));
	vec3 R = (reflect(lightDir, normalize(vertNormal)).xyz); //Reflect light vector.
	vec3 Ambient = kA.xyz * iA; //Ambient light.
	//Get lambertian Term.
	vec3 Diffuse = nDl * kD.xyz * iD;
	if(textureUsed == 0){
		nDl = max(0.0f, dot(normalize(vertNormal), -lightDir));
	}
	else if(textureUsed == 1){
		//Get texture data from UV coords.
		vec4 diffuseTextureData = texture(DiffuseTexture, vertUV);
		vec4 specularTextureData =  texture(SpecularTexture, vertUV);
		vec4 normalTextureData = texture(NormalTexture, vertUV);
		nDl = max(0.0f, dot(normalize(normalTextureData), -lightDir));
		R = (reflect(lightDir, normalize(normalTextureData)).xyz); //Reflect light vector.
		Ambient = diffuseTextureData.rgb + kA.xyz * iA; //Ambient light.
		Ambient = Ambient / 2.0f;
		//Get lambertian Term.
		Diffuse = ((nDl * kD.xyz * iD) + (diffuseTextureData.rgb * nDl)) / 2.0f;
	}
	
	//	vec4 textureData = diffuseTextureData + specularTextureData + normalTextureData;
	//	textureData = textureData;

	vec3 E = normalize(camPos - vertPos).xyz; //Surface to eye vector.

	float specTerm = pow(max(0.0f, dot(E, R)), kS.a) * ((lightStrength / 200.0f)); //Specular Term.
	vec3 specular = ((kS.xyz * iS * specTerm));

	//Limit vert colour to the max value of output rgb values.
	vec4 vertColour = vec4(Ambient + Diffuse + specular, 1.0f);
	vec4 litColour = vec4(vertColour.xyz * nDl, 1.0);
	vec4 colourOutput = ((vertColour + (litColour)) * ((lightStrength / 100.0f)));

	outputColour = colourOutput;
}