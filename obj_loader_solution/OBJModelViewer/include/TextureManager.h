#pragma once
#include <map>
#include <string>
//Forward declare Texture as we only need to keep a pointer here.
//This avoids cyclic dependency.
class Texture;

class TextureManager
{
public:
	//This manager class will act as a singleton object for ease of access.s
	static TextureManager* CreateInstance();
	static TextureManager* GetInstance();
	static void DestroyInstance();

	bool TextureExists(const char* a_pName);
	//Load a texture from file --> calls Texture::Load().
	unsigned int LoadTexture(const char* a_pfileName);
	unsigned int GetTexture(const char* a_fileName);

	void ReleaseTexture(unsigned int a_texture);

private:
	static TextureManager* m_instance;

	//A small structure to reference count a texture.
	//References count indicates how many pointers are
	//currently pointing to this texture -> only unload at 0 refs.
	typedef struct TextureRef
	{
		Texture* pTexture;
		unsigned int refCount;
	}TextureRef;

	std::map<std::string, TextureRef> m_pTextureMap;

	TextureManager();
	~TextureManager();
};