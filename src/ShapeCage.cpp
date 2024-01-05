#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include "ShapeCage.h"
#include "GLSL.h"
#include "Grid.h"

using namespace std;

ShapeCage::ShapeCage() :
	posBufID(0),
	texBufID(0),
	elemBufID(0),
    tileBufID(1),
    cpsBufID(0)
{
}

ShapeCage::~ShapeCage()
{
}

void ShapeCage::load(const string &meshName)
{
	// Load geometry
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if(!rc) {
		cerr << errStr << endl;
	} else {
		posBuf = attrib.vertices;
		norBuf = attrib.normals;
		texBuf = attrib.texcoords;
		//assert(posBuf.size() == norBuf.size());
		// Loop over shapes
		for(size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			const tinyobj::mesh_t &mesh = shapes[s].mesh;
			size_t index_offset = 0;
			for(size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
				size_t fv = mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for(size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = mesh.indices[index_offset + v];
					elemBuf.push_back(idx.vertex_index);
				}
				index_offset += fv;
				// per-face material (IGNORE)
				//shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void ShapeCage::toLocal()
{
	// Find which tile each vertex belongs to.
	// Store (row, col) into tileBuf.
	int nVerts = (int)posBuf.size()/3;
	posLocalBuf.resize(nVerts*2);
	tileIndexBuf.resize(nVerts);
	int nrows = grid->getRows();
	int ncols = grid->getCols();
	// Go through all vertices
    for(int k = 0; k < nVerts; ++k){
        for(int col = 0; col < ncols-1; ++col) {
            for(int row = 0; row < nrows-1; ++row) {
                // Get the four control points for corresponding to (row, col)
                int tileIndex = grid->indexAt(row, col);
                vector<glm::vec2> cps = grid->getTileCPs(tileIndex);
                
                float xmin=1,xmax=-1,ymin=1,ymax=-1;
                for(auto cp:cps){
                    if(xmin>cp.x)xmin=cp.x;
                    if(xmax<cp.x)xmax=cp.x;
                    if(ymin>cp.y)ymin=cp.y;
                    if(ymax<cp.y)ymax=cp.y;
                }
                //            cout<<xmin<<xmax<<ymin<<ymax<<tileIndex<<nVerts<<endl;
//                vector<int> a={tileIndex, tileIndex+1, tileIndex+ncols, tileIndex+ncols+1};
                //            for(int k :a){
                float x = posBuf[3*k];
                float y = posBuf[3*k+1];
                if(x>=xmax||x<xmin||y>=ymax||y<ymin)continue;
                float u=(x-xmin)/(xmax-xmin);
                float v=(y-ymin)/(ymax-ymin);
                x=u,y=v;
                
                posLocalBuf[2*k+0] = x;
                posLocalBuf[2*k+1] = y;
                tileIndexBuf[k] = tileIndex;
            }
        }
    }
    glGenBuffers(1, &posBufID);
    glBindBuffer(GL_ARRAY_BUFFER, posBufID);
//    glBufferData(GL_ARRAY_BUFFER, posLocalBuf.size()*sizeof(float), &posLocalBuf[0], GL_DYNAMIC_DRAW);
//    
//    glGenBuffers(1, &tileBufID);
//    glBindBuffer(GL_ARRAY_BUFFER, tileBufID);
    glBufferData(GL_ARRAY_BUFFER, tileIndexBuf.size()*sizeof(float), &tileIndexBuf[0], GL_DYNAMIC_DRAW);
    
//    for(int k = 0; k < nVerts; ++k) {
//        float x = posBuf[3*k];
//        float y = posBuf[3*k+1];
//        //
//        // IMPLEMENT ME
//        // Fill in posLocalBuf and tileIndexBuf.
//        vector<glm::vec2> cps = grid->getTileCPs(0);
//        float xmin=-1,xmax=1,ymin=-1,ymax=1;
//        for(auto cp:cps){
//            if(xmin>cp.x)xmin=cp.x;
//            if(xmax<cp.x)xmax=cp.x;
//            if(ymin>cp.y)ymin=cp.y;
//            if(ymax<cp.y)ymax=cp.y;
//        }
//        float u=(x-xmin)/(xmax-xmin);
//        float v=(y-ymin)/(ymax-ymin);
//        x=u,y=v;
//        //
//        posLocalBuf[2*k+0] = x;
//        posLocalBuf[2*k+1] = y;
//        tileIndexBuf[k] = 0;
//    }
}

void ShapeCage::toWorld()
{
	/*int nVerts = (int)posBuf.size()/3;
	for(int k = 0; k < nVerts; ++k) {
		float u = posLocalBuf[2*k];
		float v = posLocalBuf[2*k+1];
		//
		// IMPLEMENT ME
		// Fill in posBuf from the info stored in posLocalBuf and tileIndexBuf.
        vector<glm::vec2> cps = grid->getTileCPs(tileIndexBuf[k]);
        glm::vec2 p((1-v)*((1-u)*cps[0]+u*cps[1])+v*((1-u)*cps[2]+u*cps[3]));
        u=p.x,v=p.y;
		//
		posBuf[3*k+0] = u;
		posBuf[3*k+1] = v;
	}
	// Send the updated world position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_DYNAMIC_DRAW);*/
}

void ShapeCage::init()
{
	// Send the texture coordinates array (if it exists) to the GPU
	if(!texBuf.empty()) {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	} else {
		texBufID = 0;
	}
	
	// Send the index array to the GPU
	glGenBuffers(1, &elemBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elemBuf.size()*sizeof(unsigned int), &elemBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void ShapeCage::draw(int h_pos, int h_tex, int h_tile) const
{
	// Enable and bind position array for drawing
	glEnableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Enable and bind texcoord array for drawing
	glEnableVertexAttribArray(h_tex);
	glBindBuffer(GL_ARRAY_BUFFER, texBufID);
	glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Enable and bind position array for drawing
//    glEnableVertexAttribArray(h_tile);
//    glBindBuffer(GL_ARRAY_BUFFER, tileBufID);
//    glVertexAttribPointer(h_tile, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Bind element array for drawing
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elemBufID);
	
	// Draw
	int nElements = (int)elemBuf.size();
	glDrawElements(GL_TRIANGLES, nElements, GL_UNSIGNED_INT, 0);
	
	// Disable and unbind
//    glDisableVertexAttribArray(h_tile);
	glDisableVertexAttribArray(h_tex);
	glDisableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
