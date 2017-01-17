#ifndef SpriteRenderer_h__
#define SpriteRenderer_h__

#include <string>
#include <vector>

#include "contentManager.h"
#include "characterSet.h"

#include "rect.h"
#include "glDrawBinds.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <GL/glew.h>

class Texture;

/**
* A sprite renderer. It renders sprites.
*
* All draw call must be submitted between Begin() and End()!
*
* Origin is in the top-left corner of the screen
*/
class SpriteRenderer
{
public:
	SpriteRenderer();
	~SpriteRenderer();

	bool Init(ContentManager& contentManager, int width, int height);

	void Begin();
	void End();

	void DrawString(const CharacterSet* characterSet, const std::string& text, glm::vec2 position, int maxWidth, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	/**
	* Draws the given string at the given position
	* 
	* \param characterSet
	* \param text
	* \param position
	* \param color
	*
	* \returns cursor position after drawing
	*/
	glm::vec2 DrawString(const CharacterSet* characterSet, const std::string& text, glm::vec2 position, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	/**
	* Draws a portion of \p text at the given position
	* 
	* \param characterSet
	* \param text
	* \param position
	* \param startIndex index of first character to draw
	* \param count number of characters to draw
	* \param color
	*
	* \returns cursor position after drawing
	*/
	glm::vec2 DrawString(const CharacterSet* characterSet, const std::string&, glm::vec2 position, unsigned int startIndex, unsigned int count, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	/**
	* Draws \p texture2D into \p drawRect
	* 
	* \param texture2D texture to draw
	* \param drawRect rectangle to draw into
	* \param color color to tint texture with
	*/
	void Draw(const Texture& texture2D, const Rect& drawRect, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	/**
	* Draws a portion of \p texture2D
	* 
	* \param texture2D texture to draw
	* \param position where to draw the texture
	* \param clipRect which portion of the texture to draw (in texels)
	* \param color color to tint the texture with
	*/
	void Draw(const Texture& texture2D, glm::vec2 position, const Rect& clipRect, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	/**
	* Draws a portion of \p texture2D into \p position
	* 
	* \param texture2D texture to draw
	* \param position rectangle to draw the texture into
	* \param clipRect which portion of the texture to draw (in texels)
	* \param color color to tint the texture with
	*
	* \returns
	*/
	void Draw(const Texture& texture2D, const Rect& position, const Rect& clipRect, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	/**
	* Draws a single-colored rectangle onto the screen
	* 
	* \param drawRect rectangle (in texels) where to draw
	* \param color color of rectangle
	*/
	void Draw(const Rect& drawRect, glm::vec4 color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	
	/**
	* Actually does drawing.
	*/
	void Draw();

	/**
	* Doesn't actually do anything
	*/
	void EnableScissorTest(const Rect& region);
	/**
	* Doesn't actually do anything
	*/
	void DisableScissorTest();

	/**
	* Returns whether or not there is anything lying in the buffers, ready to be drawn
	*/
	bool AnythingToDraw() const;
private:
	struct BatchData
	{
		BatchData()
				: positionMin(0, 0)
				  , positionMax(0, 0)
				  , texCoordsMin(0, 0)
				  , texCoordsMax(0, 0)
				  , color(0, 0, 0, 0)
		{ };
		BatchData(glm::vec2 positionMin
				  , glm::vec2 positionMax
				  , glm::vec2 texCoordsMin
				  , glm::vec2 texCoordsMax
				  , glm::vec4 color)
				: positionMin(positionMin)
				  , positionMax(positionMax)
				  , texCoordsMin(texCoordsMin)
				  , texCoordsMax(texCoordsMax)
				  , color(color)
		{ };
		~BatchData()
		{ };

		glm::vec2 positionMin;
		glm::vec2 positionMax;

		glm::vec2 texCoordsMin;
		glm::vec2 texCoordsMax;

		glm::vec4 color;
	};

	struct SpriteBatch
	{
		SpriteBatch()
				: texture(0)
				  , offset(0)
				  , size(0)
		{ }
		SpriteBatch(GLuint texture)
				: texture(texture)
				  , offset(0)
				  , size(0)
		{ }

		~SpriteBatch() = default;

		GLuint texture;

		unsigned int offset;
		unsigned int size;
	};

	struct Vertex2D
	{
		Vertex2D()
				: position(0.0f, 0.0f)
				  , texCoords(0.0f, 0.0)
				  , color(0.0f, 0.0f, 0.0f, 0.0f)
		{ }
		Vertex2D(glm::vec2 position, glm::vec2 texCoords, glm::vec4 color)
				: position(position)
				  , texCoords(texCoords)
				  , color(color)
		{ }
		Vertex2D(float posX, float posY, float texCoordsX, float texCoordsY, glm::vec4 color)
				: position(posX, posY)
				  , texCoords(texCoordsX, texCoordsY)
				  , color(color)
		{ }
		~Vertex2D() = default;

		glm::vec2 position; //8 bytes
		glm::vec2 texCoords; //8 bytes
		glm::vec4 color; //16 bytes
	};

	struct SpriteInfo
	{
		SpriteInfo()
		{ };
		SpriteInfo(Rect position, Rect clipRect, glm::vec4 color)
				: position(position)
				  , clipRect(clipRect)
				  , color(color)
		{ };
		~SpriteInfo()
		{ };

		Rect position;
		Rect clipRect;
		glm::vec4 color;
	};

	void AddNewBatch(const Texture& newTexture);
	void AddDataToBatch(const BatchData& data);

	bool hasBegun;

	//D3D11_RECT defaultScissorRect;

	//////////////////////////////////////////////////////////////////////////
	//BUFFER VARIABLES
	//////////////////////////////////////////////////////////////////////////
	//ID3D11DeviceContext* deviceContext;

	//VertexShader* vertexShader;
	//PixelShader* pixelShader;
	glm::mat4x4 viewProjectionMatrix;

	GLVertexBuffer vertexBuffer;
	GLIndexBuffer indexBuffer;

    GLDrawBinds drawBinds;

	/**
	* Max inserts per batch. 1 batch = 1 draw call
	*/
	const static unsigned int MAX_BUFFER_INSERTS = 8192;

	//Vertex2D = 32 bytes (with padding)
	const static unsigned int MAX_VERTEX_BUFFER_INSERTS = MAX_BUFFER_INSERTS * 4;
	const static unsigned int VERTEX_BUFFER_SIZE = MAX_VERTEX_BUFFER_INSERTS * sizeof(Vertex2D);
	const static unsigned int MAX_INDEX_BUFFER_INSERTS = MAX_BUFFER_INSERTS * 6;
	const static unsigned int INDEX_BUFFER_SIZE = MAX_INDEX_BUFFER_INSERTS * sizeof(unsigned int);

	unsigned int bufferInserts;

	std::vector<SpriteBatch> spriteBatch;

	std::vector<Vertex2D> vertices;
	std::vector<GLuint> indicies;

	//For easy drawing of rectangles via Draw()
	Texture* whiteTexture;
	Rect whiteTextureClipRect;

	GLuint currentTexture;
};

#endif // SpriteRenderer_h__
