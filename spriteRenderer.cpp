#include "spriteRenderer.h"

#include "logger.h"

#include "content/textureCreationParameters.h"
#include "content/shaderContentParameters.h"

#include "content/memoryTexture.h"
#include "content/textureCreationParameters.h"

#include <glm/gtc/matrix_transform.hpp>

SpriteRenderer::SpriteRenderer()
	: hasBegun(false)
	, bufferInserts(-1)
	, whiteTexture(nullptr)
	, currentTexture(0)
{
}

SpriteRenderer::~SpriteRenderer()
{
}

bool SpriteRenderer::Init(ContentManager& contentManager, int screenWidth, int screenHeight)
{
	hasBegun = false;

    SetScreenSize(screenWidth, screenHeight);

	//RGBA RGBA
	//RGBA RGBA
	unsigned char whiteTextureData[] =
	{
		255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255
	};

	TextureCreationParameters whiteTextureParameters("whiteTexture", 2, 2, GLEnums::INTERNAL_FORMAT::RGB8, GLEnums::FORMAT::RGBA, GLEnums::TYPE::UNSIGNED_BYTE, &whiteTextureData);
	whiteTexture = contentManager.Load<MemoryTexture>("", &whiteTextureParameters);
	if(whiteTexture == nullptr)
	{
		Logger::LogLine(LOG_TYPE::FATAL, "Couldn't create white texture from memory");
		return false;
	}


	////////////////////////////////////////////////////////////
	//Create buffers
	////////////////////////////////////////////////////////////
    vertexBuffer.Init<glm::vec2, glm::vec2, glm::vec4>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, MAX_VERTEX_BUFFER_INSERTS);
    indexBuffer.Init<GLuint>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, MAX_INDEX_BUFFER_INSERTS);

	//////////////////////////////////////////////////////////////////////////
	//Shaders
	//////////////////////////////////////////////////////////////////////////
    drawBinds.AddShaders(contentManager
                     , GLEnums::SHADER_TYPE::VERTEX, "rendering/spriteVertex.glsl"
                     , GLEnums::SHADER_TYPE::FRAGMENT, "rendering/spritePixel.glsl");

    drawBinds.AddBuffers(&vertexBuffer, &indexBuffer);

    drawBinds.AddUniform("viewProjMatrix", viewProjectionMatrix);

    if(!drawBinds.Init())
		return false;

	return true;
}

void SpriteRenderer::SetScreenSize(int width, int height)
{
    viewProjectionMatrix = glm::ortho(0.0f, (float)width, (float)height, 0.0f);
}

void SpriteRenderer::Begin()
{
	if(hasBegun)
		Logger::LogLine(LOG_TYPE::WARNING, "SpriteRenderer::Begin called twice in a row");

	//////////////////////////////////////////////////
	//Set
	//////////////////////////////////////////////////

	hasBegun = true;

	bufferInserts = 0;
}

void SpriteRenderer::End()
{
	if(!hasBegun)
		Logger::LogLine(LOG_TYPE::WARNING, "SpriteRenderer::End called without begin");

	if(spriteBatch.size() > 0)
		Draw();

	//Unbind
	drawBinds.Unbind();

	hasBegun = false;
}

void SpriteRenderer::Draw(const Texture& texture2D, const Rect& drawRect, glm::vec4 color)
{
    Draw(texture2D, drawRect, Rect(0.0f, 0.0f, static_cast<float>(texture2D.GetWidth()), static_cast<float>(texture2D.GetHeight())), color);
}

void SpriteRenderer::Draw(const Texture& texture2D, glm::vec2 position, const Rect& clipRect, glm::vec4 color)
{
    const glm::vec2 prediv(texture2D.GetPredivWidth(), texture2D.GetPredivHeight());

    glm::vec2 texCoordsMax = position + glm::vec2(clipRect.GetWidth(), clipRect.GetHeight());
    glm::vec2 clipMin = clipRect.GetMinPosition() * prediv;
    glm::vec2 clipMax = clipRect.GetMaxPosition() * prediv;

	if(currentTexture != texture2D.GetTexture())
		AddNewBatch(texture2D);

	AddDataToBatch(BatchData(
				position
				, texCoordsMax
				, clipMin
				, clipMax
				, color));
	}

void SpriteRenderer::Draw(const Texture& texture2D, const Rect& position, const Rect& clipRect, glm::vec4 color)
{
	glm::vec2 tempFloat2 = clipRect.GetMinPosition();
	glm::vec2 predivSize = glm::vec2(texture2D.GetPredivWidth(), texture2D.GetPredivHeight());

    glm::vec2 clipMin = clipRect.GetMinPosition() * predivSize;
    glm::vec2 clipMax = clipRect.GetMaxPosition() * predivSize;

	if(currentTexture != texture2D.GetTexture())
		AddNewBatch(texture2D);

	AddDataToBatch(BatchData(
		position.GetMinPosition()
		, position.GetMaxPosition()
		, clipMin
		, clipMax
		, color));
	}

void SpriteRenderer::Draw(const Rect& drawRect, glm::vec4 color /*= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)*/)
{
	if(currentTexture != whiteTexture->GetTexture())
		AddNewBatch(*whiteTexture);

	AddDataToBatch(BatchData(
		drawRect.GetMinPosition()
		, drawRect.GetMaxPosition()
		, glm::vec2(0.0f, 0.0f)
		, glm::vec2(1.0f, 1.0f)
		, color));
}

void SpriteRenderer::Draw()
{
    // TODO
//	vertexShader->Bind(deviceContext);
//	pixelShader->Bind(deviceContext);
//
//	deviceContext->RSSetState(RasterizerStates::solidScissor->Get());
//	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	float blendFactors[] = { 0.0f, 0.0f, 0.0f, 0.0f };
//	deviceContext->OMSetBlendState(BlendStates::singleDefault->Get(), blendFactors, 0xFFFFFFFF);
//	deviceContext->OMSetDepthStencilState(DepthStencilStates::off->Get(), 0xFFFFFFFF);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    indexBuffer.Update(indicies);
    vertexBuffer.Update(&vertices[0], sizeof(Vertex2D) * vertices.size());

    drawBinds.Bind();
    drawBinds["viewProjMatrix"] = viewProjectionMatrix;

	AddNewBatch(*whiteTexture);
	spriteBatch.pop_back();

    //GLBufferLock vertexLock(vertexBuffer);
    //GLBufferLock indexLock(indexBuffer);

	for(const SpriteBatch& batch : spriteBatch)
	{
		//deviceContext->PSSetShaderResources(0, 1, &batch.textureResourceView)-;
		//deviceContext->DrawIndexed(batch.size, batch.offset, 0);
		glBindTexture(GL_TEXTURE_2D, batch.texture);

        drawBinds.DrawElements(batch.size, batch.offset);
	}

	vertices.clear();
	indicies.clear();

	spriteBatch.clear();
	currentTexture = 0;

	bufferInserts = 0;

    drawBinds.Unbind();
}

void SpriteRenderer::EnableScissorTest(const Rect& region)
{
	/*if(bufferInserts > 0)
		Draw();

	D3D11_RECT rect;
	rect.left = static_cast<LONG>(region.GetMinPosition().x);
	rect.top = static_cast<LONG>(region.GetMinPosition().y);
	rect.right = static_cast<LONG>(region.GetMaxPosition().x);
	rect.bottom = static_cast<LONG>(region.GetMaxPosition().y);

	deviceContext->RSSetScissorRects(1, &rect);*/

	//glScissor(static_cast<GLint>(region.GetMinPosition().x), static_cast<GLint>(resolutionRect.GetHeight() - region.GetMinPosition().y - region.GetHeight()), static_cast<GLsizei>(region.GetWidth()), static_cast<GLsizei>(region.GetHeight()));
	//glEnable(GL_SCISSOR_TEST);
}

void SpriteRenderer::DisableScissorTest()
{
	//if(bufferInserts > 0)
	//	Draw();

	//deviceContext->RSSetScissorRects(1, &defaultScissorRect);

	//glDisable(GL_SCISSOR_TEST);
}

bool SpriteRenderer::AnythingToDraw() const
{
	return bufferInserts > 0;
}

void SpriteRenderer::AddNewBatch(const Texture& newTexture)
{
	currentTexture = newTexture.GetTexture();

	if(spriteBatch.size() == 1)
	{
		spriteBatch.back().size = static_cast<unsigned int>(indicies.size());
	}
	else if(spriteBatch.size() > 1)
	{
		spriteBatch.back().offset = spriteBatch[spriteBatch.size() - 2].offset + spriteBatch[spriteBatch.size() - 2].size;
		spriteBatch.back().size = static_cast<unsigned int>(indicies.size() - spriteBatch.back().offset);
	}

	spriteBatch.emplace_back(newTexture.GetTexture());
}

void SpriteRenderer::AddDataToBatch(const BatchData& data)
{
#ifdef DETAILED_GRAPHS
	Timer timer;
	timer.Start();
#endif // DETAILED_GRAPHS

	//Top left
	vertices.emplace_back(data.positionMin, data.texCoordsMin, data.color);

	//Top right
	vertices.emplace_back(data.positionMax.x, data.positionMin.y, data.texCoordsMax.x, data.texCoordsMin.y, data.color);

	//Bottom right
	vertices.emplace_back(data.positionMax, data.texCoordsMax, data.color);

	//Bottom left
	vertices.emplace_back(data.positionMin.x, data.positionMax.y, data.texCoordsMin.x, data.texCoordsMax.y, data.color);

	//ELEMENT BUFFER
	indicies.emplace_back(bufferInserts * 4); //Top left
	indicies.emplace_back(bufferInserts * 4 + 3); //Bottom left
	indicies.emplace_back(bufferInserts * 4 + 2); //Bottom right
	indicies.emplace_back(bufferInserts * 4 + 2); //Bottom right
	indicies.emplace_back(bufferInserts * 4 + 1); //Top right
	indicies.emplace_back(bufferInserts * 4); //Top left

#ifdef DETAILED_GRAPHS
	timer.Stop();
	addDataToBatchTime += timer.GetTimeMillisecondsFraction();
#endif // DETAILED_GRAPHS

	bufferInserts++;
	if(bufferInserts == MAX_BUFFER_INSERTS)
		Draw();
}

void SpriteRenderer::DrawString(const CharacterSet* characterSet, const std::string& text, glm::vec2 position, int maxWidth, glm::vec4 color /*= glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)*/)
{
	//This method would use an unsigned int for maxWidth but workarounds/hacks should allow a negative width on characters.
	//Besides, 0x7FFFFFFF should be plenty for maxWidth
	int currentWidth = 0;

	for(int i = 0, end = static_cast<int>(text.size()); i < end; ++i)
	{
		const Character* character = characterSet->GetCharacter(text[i]);

		if(currentWidth + character->xAdvance > maxWidth)
			return;

		glm::vec2 drawPosition(position.x + character->xOffset, position.y + characterSet->GetLineHeight() - character->yOffset);

		Draw(*characterSet->GetTexture(), drawPosition, Rect(character->x, character->y, character->width, character->height), color);
		position.x += character->xAdvance;

		currentWidth += character->xAdvance;
	}
}

glm::vec2 SpriteRenderer::DrawString(const CharacterSet* characterSet, const std::string& text, glm::vec2 position, glm::vec4 color)
{
	for(int i = 0, end = static_cast<int>(text.size()); i < end; ++i) //TODO: Optimize, bulk draw instead of calling Draw( ... )
	{
		const Character* character = characterSet->GetCharacter(text[i]);

		glm::vec2 drawPosition(position.x + character->xOffset, position.y + characterSet->GetLineHeight() - character->yOffset);

		Draw(*characterSet->GetTexture(), drawPosition, Rect(character->x, character->y, character->width, character->height), color);
		position.x += character->xAdvance;
	}

	return position;
}

glm::vec2 SpriteRenderer::DrawString(const CharacterSet* characterSet, const std::string& text, glm::vec2 position, unsigned int startIndex, unsigned int count, glm::vec4 color)
{
	for(int i = startIndex, end = startIndex + count; i < end; ++i)
	{
		const Character* character = characterSet->GetCharacter(text[i]);

		glm::vec2 drawPosition(position.x + character->xOffset, position.y + characterSet->GetLineHeight() - character->yOffset);

		Draw(*characterSet->GetTexture(), drawPosition, Rect(character->x, character->y, character->width, character->height), color);
		position.x += character->xAdvance;
	}

	return position;
}
