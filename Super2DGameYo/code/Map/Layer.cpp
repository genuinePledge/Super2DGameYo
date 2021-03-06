#include "Layer.h"

Layer::Layer(int z)
	: z(z)
{
}

Layer::~Layer()
{
}

void Layer::parse(const pugi::xml_node& node)
{
	auto properties = node.child("properties").children();

	for (auto custom_prop : properties)
	{
		if (!std::string("tileset_id").compare(custom_prop.attribute("name").value()))
			m_tilesetID = atoi(custom_prop.attribute("value").value());
		if (!std::string("is_static").compare(custom_prop.attribute("name").value()))
			is_static = atoi(custom_prop.attribute("value").value());
	}

	pugi::xml_node data = node.child("data");
	std::stringstream ss(data.text().get());
	std::string str;

	m_name = node.attribute("name").value();
	m_width = atoi(node.attribute("width").value());
	m_height = atoi(node.attribute("height").value());

	while (std::getline(ss, str, ','))
		m_data.push_back(atoi(str.c_str()));
}

void Layer::setTileset(std::vector<Tileset>& tilesets)
{
	m_tileset = tilesets[m_tilesetID];
	if (m_tilesetID > 0)
	{
		int offset = 0;
		for (uint8_t i = 0; i < m_tilesetID; i++)
			offset += tilesets[i].getTilecount();
		offsetTexCoords(offset);
	}
}

void Layer::offsetTexCoords(int offset)
{
	for (auto& i : m_data)
	{
		if (!i)
			continue;
		i -= offset;
	}
}

void Layer::initVertexArray()
{
	m_vertices.setPrimitiveType(sf::Quads);
	m_vertices.resize(m_width * m_height * 4);

	for (int i = 0; i < m_width; i++)
		for (int j = 0; j < m_height; j++)
		{
			int tileNumber = m_data[i + j * m_width];
			sf::Vertex* quad = &m_vertices[(i + j * m_width) * 4];
			
			if (tileNumber == 0)
			{
				quad[0].position = sf::Vector2f(i * m_tilesize, j * m_tilesize);
				quad[1].position = sf::Vector2f((i + 1) * m_tilesize, j * m_tilesize);
				quad[2].position = sf::Vector2f((i + 1) * m_tilesize, (j + 1) * m_tilesize);
				quad[3].position = sf::Vector2f(i * m_tilesize, (j + 1) * m_tilesize);

				quad[0].color = sf::Color(255, 255, 255, 0);
				quad[1].color = sf::Color(255, 255, 255, 0);
				quad[2].color = sf::Color(255, 255, 255, 0);
				quad[3].color = sf::Color(255, 255, 255, 0);
			}
			else
			{
				sf::IntRect texrect = m_tileset.texCoords[tileNumber];

				quad[0] = sf::Vertex(sf::Vector2f(i * m_tilesize, j * m_tilesize),			  sf::Vector2f(texrect.left, texrect.top));
				quad[1] = sf::Vertex(sf::Vector2f((i + 1) *m_tilesize, j * m_tilesize),		  sf::Vector2f(texrect.left + texrect.width, texrect.top));
				quad[2] = sf::Vertex(sf::Vector2f((i + 1) *m_tilesize, (j + 1) * m_tilesize), sf::Vector2f(texrect.left + texrect.width, texrect.top + texrect.height));
				quad[3] = sf::Vertex(sf::Vector2f(i * m_tilesize, (j + 1) * m_tilesize),	  sf::Vector2f(texrect.left, texrect.top + texrect.height));
			}
		}
}

void Layer::updateVertexArray(int changedTile, int framecount)
{
	for (int i = 0; i < m_width; i++)
	{
		for (int j = 0; j < m_height; j++)
		{
			int tileNumber = m_data[i + j * m_width];

			if (changedTile - 1 == tileNumber || (changedTile + framecount) - 1 == tileNumber)
			{
				m_data[i + j * m_width] = changedTile;

				sf::Vertex* quad = &m_vertices[(i + j * m_width) * 4];

				sf::IntRect texrect = m_tileset.texCoords[changedTile];

				quad[0].texCoords = sf::Vector2f(texrect.left, texrect.top);
				quad[1].texCoords = sf::Vector2f(texrect.left + texrect.width, texrect.top);
				quad[2].texCoords = sf::Vector2f(texrect.left + texrect.width, texrect.top + texrect.height);
				quad[3].texCoords = sf::Vector2f(texrect.left, texrect.top + texrect.height);
			}
		}
	}
}

void Layer::setTilesize(int size)
{
	m_tilesize = size;
}

std::string Layer::getName() const
{
	return m_name;
}

bool Layer::isStatic() const
{
	return is_static;
}

Tileset Layer::getTileset()
{
	return m_tileset;
}

void Layer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = &m_tileset.m_texture;
	target.draw(m_vertices, states);
}
