/*
This file is part of the graphics core library.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* fundamendal data structures for 3D graphics
 *
 * Author: John Tsiombikas 2004
 * Modified: 
 * 		Mihalis Georgoulopoulos 2004
 * 		John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#include <iostream>
#include <cstdlib>
#include <cfloat>
#include "3dgeom.hpp"
#include "common/heapsort.hpp"

#ifdef USING_3DENGFX
#include "3dengfx/3denginefx.hpp"
#endif	// USING_3DENGFX

using std::vector;
using namespace glext;

TexCoord::TexCoord(scalar_t u, scalar_t v, scalar_t w) {
	this->u = u;
	this->v = v;
	this->w = w;
}

// Vertex class implementation

Vertex::Vertex() {
	//normal = Vector3(0, 1, 0);
}

Vertex::Vertex(const Vector3 &position, scalar_t tu, scalar_t tv, const Color &color) {
	pos = position;
	normal = Vector3(0, 1, 0);
	tex[0].u = tex[1].u = tu;
	tex[0].v = tex[1].v = tv;
	this->color = color;
}

/////////// Edge class implementation ///////////

Edge::Edge() {
	vertices[0] = vertices[1] = 0;
	adjfaces[0] = adjfaces[1] = NO_ADJFACE;
}

Edge::Edge(unsigned long v1, unsigned long v2, unsigned long af1, unsigned long af2) {
	vertices[0] = v1;
	vertices[1] = v2;
	adjfaces[0] = af1;
	adjfaces[1] = af2;
}

/////////// Triangle class implementation /////////////
Triangle::Triangle(Index v1, Index v2, Index v3) {
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;

	smoothing_group = 0;
}

void Triangle::calculate_normal(const Vertex *vbuffer, bool normalize) {
	Vector3 v1 = vbuffer[vertices[1]].pos - vbuffer[vertices[0]].pos;
	Vector3 v2 = vbuffer[vertices[2]].pos - vbuffer[vertices[0]].pos;
	normal = cross_product(v1, v2);
	if(normalize) normal.normalize();
}

Quad::Quad(Index v1, Index v2, Index v3, Index v4) {
	vertices[0] = v1;
	vertices[1] = v2;
	vertices[2] = v3;
	vertices[3] = v4;
}

void Quad::calculate_normal(const Vertex *vbuffer, bool normalize) {
	Vector3 v1 = vbuffer[vertices[1]].pos - vbuffer[vertices[0]].pos;
	Vector3 v2 = vbuffer[vertices[2]].pos - vbuffer[vertices[0]].pos;
	normal = cross_product(v1, v2);
	if(normalize) normal.normalize();
}

///////////////////////////////////////////
// Index specialization of GeometryArray //
///////////////////////////////////////////

GeometryArray<Index>::GeometryArray(bool dynamic) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	vbo_in_sync = false;

	set_dynamic(dynamic);
}

GeometryArray<Index>::GeometryArray(const Index *data, unsigned long count, bool dynamic) {
	this->data = 0;
	this->count = 0;
	buffer_object = INVALID_VBO;
	set_dynamic(dynamic);

	set_data(data, count);
}

void tri_to_index_array(GeometryArray<Index> *ia, const GeometryArray<Triangle> &ta) {
	ia->dynamic = ta.get_dynamic();
	ia->data = 0;
	ia->count = 0;
	ia->buffer_object = INVALID_VBO;

	unsigned long tcount = ta.get_count();
	Index *tmp_data = new Index[tcount * 3];

	Index *ptr = tmp_data;
	for(unsigned long i=0; i<tcount; i++) {
		for(int j=0; j<3; j++) {
			*ptr++ = ta.get_data()[i].vertices[j];
		}
	}

	ia->set_data(tmp_data, tcount * 3);
	delete [] tmp_data;
}

GeometryArray<Index>::GeometryArray(const GeometryArray<Triangle> &tarray) {
	tri_to_index_array(this, tarray);
}

GeometryArray<Index>::GeometryArray(const GeometryArray<Index> &ga) {
	data = 0;
	count = 0;
	buffer_object = INVALID_VBO;
	dynamic = ga.dynamic;

	set_data(ga.data, ga.count);
}

GeometryArray<Index>::~GeometryArray() {
	if(data) delete [] data;
#ifdef USING_3DENGFX
	if(buffer_object != INVALID_VBO) {
		glDeleteBuffers(1, &buffer_object);
	}
#endif	// USING_3DENGFX
}

GeometryArray<Index> &GeometryArray<Index>::operator =(const GeometryArray<Index> &ga) {
	dynamic = ga.dynamic;
	if(data) delete [] data;

	set_data(ga.data, ga.count);

	return *this;
}

void GeometryArray<Index>::sync_buffer_object() {
#ifdef USING_3DENGFX
	if(dynamic) return;

	if(buffer_object == INVALID_VBO) {
		glGenBuffers(1, &buffer_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer_object);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER_ARB, count * sizeof(Index), data, GL_STATIC_DRAW_ARB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	} else {

		int glerr;
		while((glerr = glGetError()) != GL_NO_ERROR) {
			std::cerr << get_glerror_string(glerr) << " ";
		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer_object);
		Index *ptr = (Index*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
		
		memcpy(ptr, data, count * sizeof(Index));
			
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
#endif	// USING_3DENGFX
	vbo_in_sync = true;

}

void GeometryArray<Index>::set_data(const Index *data, unsigned long count) {
	if(!data) return;
	if(!this->data || count != this->count) {
		if(this->data) {
			delete [] this->data;
		}
		this->data = new Index[count];
	}

	memcpy(this->data, data, count * sizeof(Index));

#ifdef USING_3DENGFX
	if(!dynamic) {
		if(buffer_object != INVALID_VBO && count != this->count) {
			glDeleteBuffers(1, &buffer_object);
		}
		sync_buffer_object();
		vbo_in_sync = true;
	}
#endif	// USING_3DENGFX

	this->count = count;
}


///////////// Triangle Mesh Implementation /////////////
TriMesh::TriMesh() {
	indices_valid = false;
	vertex_stats_valid = false;
	edges_valid = false;
}

TriMesh::TriMesh(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount) {
	indices_valid = false;
	vertex_stats_valid = false;
	edges_valid = false;
	set_data(vdata, vcount, tdata, tcount);
}

void TriMesh::calculate_edges() {
	std::cerr << "calc_edges()\n";

/*
	vector<Edge> edges;

	Triangle *tptr = tarray.get_mod_data();
	for(unsigned int i=0; i<tarray.get_count(); i++) {
		for(unsigned int j=0; j<3; j++) {	// for every edge of the triangle...
			unsigned long vindex1 = tptr->vertices[j];
			unsigned long vindex2 = tptr->vertices[(j + 1) % 3];
			bool new_edge = true;
			
			// search the list of edges for an edge with the same two vertices
			for(unsigned int k=0; k<edges.size(); k++) {
				if((edges[k].vertices[0] == vindex1 || edges[k].vertices[0] == vindex2)
						&& (edges[k].vertices[1] == vindex1 || edges[k].vertices[1] == vindex2)) {
					
					assert(edges[k].adjfaces[0] != 0xffffffff);
					assert(edges[k].adjfaces[1] == 0xffffffff);
					edges[k].adjfaces[1] = i;
					
					new_edge = false;
					break;
				}
			}

			// if we did not find this edge in the list, add a new one.
			if(new_edge) {
				edges.push_back(Edge(vindex1, vindex2, i, 0xffffffff));
			}
		}
		
		tptr++;
	}

	earray.set_data(&edges[0], edges.size());
	edges_valid = true;
*/
	
	unsigned int vcount = varray.get_count();
	vector<Edge> *edge_table = new vector<Edge>[vcount];
	const Triangle *tris = tarray.get_data();
	unsigned int tcount = tarray.get_count();
	unsigned int num_edges = 0;

	// Triangle loop
	for (unsigned int i=0; i<tcount; i++)
	{
		unsigned int a, b, temp;
		for (unsigned int j=0; j<3; j++)
		{
			a = tris[i].vertices[j];
			b = tris[i].vertices[(j + 1) % 3];

			if (a > b)
			{
				temp = b;
				b = a;
				a = temp;
			}

			int edge_found = -1;
			for (unsigned int edge = 0; edge < edge_table[a].size(); edge++)
			{
				if (edge_table[a][edge].vertices[1] == b)
				{
					edge_found = edge;
					break;
				}
			}

			if (edge_found != -1)
			{
				// edge was already in the list
				// add the second face to this edge
				edge_table[a][edge_found].adjfaces[1] = i;
			}
			else
			{
				// add a new edge to the list
				Edge new_edge(a, b, i);
				edge_table[a].push_back(new_edge);
				num_edges++;
			}
		}
	} // End triangle loop

	// collect edges
	Edge *edges = new Edge[num_edges];
	int k = 0;
	for (unsigned int i=0; i<vcount; i++)
	{
		for (unsigned int j=0; j<edge_table[i].size(); j++)
		{
			edges[k] = edge_table[i][j];
			k++;
		}
	}

	earray.set_data(edges, num_edges);
	edges_valid = true;

	// cleanup
	delete [] edge_table;
	delete [] edges;
}

const IndexArray *TriMesh::get_index_array() {
	if(!indices_valid) {
		tri_to_index_array(&iarray, tarray);
		indices_valid = true;
	}
	return &iarray;
}

const GeometryArray<Edge> *TriMesh::get_edge_array() const {
	if(!edges_valid) {
		const_cast<TriMesh*>(this)->calculate_edges();
	}
	return &earray;
}

void TriMesh::set_data(const Vertex *vdata, unsigned long vcount, const Triangle *tdata, unsigned long tcount) {
	get_mod_vertex_array()->set_data(vdata, vcount);	// also invalidates vertex stats
	get_mod_triangle_array()->set_data(tdata, tcount);	// also invalidates indices and edges
}

void TriMesh::calculate_normals() {
	// precalculate which triangles index each vertex
	std::vector<unsigned int> *tri_indices;
	tri_indices = new std::vector<unsigned int>[varray.get_count()];
	
	for(unsigned int i=0; i<tarray.get_count(); i++) {
		for(int j=0; j<3; j++) {
			tri_indices[tarray.get_data()[i].vertices[j]].push_back(i);
		}
	}
	
	// calculate the triangle normals
	for(unsigned int i=0; i<tarray.get_count(); i++) {
		tarray.get_mod_data()[i].calculate_normal(varray.get_data(), false);
	}
	
	// now calculate the vertex normals
	for(unsigned int i=0; i<varray.get_count(); i++) {
		Vector3 normal;
		for(unsigned int j=0; j<(unsigned int)tri_indices[i].size(); j++) {
			normal += tarray.get_data()[tri_indices[i][j]].normal;
		}
		normal.normalize();
		varray.get_mod_data()[i].normal = normal;
	}
	
	delete [] tri_indices;
}


void TriMesh::normalize_normals() {
	Vertex *vptr = varray.get_mod_data();
	for(unsigned int i=0; i<varray.get_count(); i++) {
		vptr[i].normal.normalize();
	}
}

/* TriMesh::invert_winding() - (JT)
 * inverts the order of vertices (cw/ccw) as well as the normals
 */
void TriMesh::invert_winding() {
	Triangle *tptr = tarray.get_mod_data();
	int tcount = tarray.get_count();

	for(int i=0; i<tcount; i++) {
		Index tmp = tptr->vertices[1];
		tptr->vertices[1] = tptr->vertices[2];
		tptr->vertices[2] = tmp;
		tptr->normal = -tptr->normal;
		tptr++;
	}

	Vertex *vptr = varray.get_mod_data();
	int vcount = varray.get_count();

	for(int i=0; i<vcount; i++) {
		vptr->normal = -vptr->normal;
		vptr++;
	}
}


void TriMesh::apply_xform(const Matrix4x4 &xform) {
	Vertex *vptr = varray.get_mod_data();
	unsigned long count = varray.get_count();

	for(unsigned long i=0; i<count; i++) {
		vptr->pos.transform(xform);
		(vptr++)->normal.transform((Matrix3x3)xform);
	}
}

void TriMesh::operator +=(const TriMesh *m2) {
	join_tri_mesh(this, this, m2);
}

/* TriMesh::sort_triangles - (MG)
 * sorts triangles according to their distance from a
 * given point (in model space).
 */
void TriMesh::sort_triangles(Vector3 point, bool hilo)
{
	const Vertex *verts = get_vertex_array()->get_data();
	unsigned int vcount = get_vertex_array()->get_count();
	Triangle *tris = get_mod_triangle_array()->get_mod_data();
	unsigned int tcount = get_triangle_array()->get_count();

	// store square distance for each vertex
	scalar_t *sq_distances = new scalar_t[vcount];

	for (unsigned int i=0; i<vcount; i++)
	{
		sq_distances[i] = (verts[i].pos - point).length_sq();
	}

	// store sum of sq distances for each triangle
	scalar_t *tri_distances = new scalar_t[tcount];

	for (unsigned int i=0; i<tcount; i++)
	{
		tri_distances[i] = 0;
		for (unsigned int j=0; j<3; j++)
		{
			tri_distances[i] += sq_distances[tris[i].vertices[j]];
		}
	}

	// sort
	sort(tris, tri_distances, tcount, hilo);
	
	// cleanup
	delete [] sq_distances;
	delete [] tri_distances;
}

VertexStatistics TriMesh::get_vertex_stats() const {
	if(!vertex_stats_valid) {
		const Vertex *varray = get_vertex_array()->get_data();
		int count = get_vertex_array()->get_count();

		const Vertex *vptr = varray;
		vstats.centroid = Vector3(0, 0, 0);
		for(int i=0; i<count; i++) {
			vstats.centroid += (vptr++)->pos;
		}
		vstats.centroid /= count;

		scalar_t min_len_sq = FLT_MAX;
		scalar_t max_len_sq = 0.0;
		scalar_t avg_len_sq = 0.0;
		
		vptr = varray;
		for(int i=0; i<count; i++) {
			scalar_t len_sq = ((vptr++)->pos - vstats.centroid).length_sq();
			if(len_sq < min_len_sq) min_len_sq = len_sq;
			if(len_sq > max_len_sq) max_len_sq = len_sq;
			avg_len_sq += len_sq;
		}

		vstats.min_dist = sqrt(min_len_sq);
		vstats.max_dist = sqrt(max_len_sq);
		vstats.avg_dist = sqrt(avg_len_sq / (scalar_t)count);
		vertex_stats_valid = true;
	}
	return vstats;
}

/* JoinTriMesh - (MG)
 * Gets 2 trimeshes and returns a new one
 * that contains both meshes
 */
void join_tri_mesh(TriMesh *ret, const TriMesh *m1, const TriMesh *m2)
{
	const Vertex *varr1 = m1->get_vertex_array()->get_data();
	const Vertex *varr2 = m2->get_vertex_array()->get_data();
	
	unsigned long vcount1 = m1->get_vertex_array()->get_count();
	unsigned long vcount2 = m2->get_vertex_array()->get_count();

	const Triangle *tarr1 = m1->get_triangle_array()->get_data();
	const Triangle *tarr2 = m2->get_triangle_array()->get_data();

	unsigned long tcount1 = m1->get_triangle_array()->get_count();
	unsigned long tcount2 = m2->get_triangle_array()->get_count();

	// allocate memory
	int vcount = vcount1 + vcount2;
	int tcount = tcount1 + tcount2;
	Vertex *varray = new Vertex[vcount];
	Triangle *tarray = new Triangle[tcount];

	// copy memory
	memcpy(varray, varr1, vcount1 * sizeof(Vertex));
	memcpy(varray + vcount1, varr2, vcount2 * sizeof(Vertex));
	memcpy(tarray, tarr1, tcount1 * sizeof(Triangle));
	memcpy(tarray + tcount1, tarr2, tcount2 * sizeof(Triangle));

	// Fix indices
	for (unsigned long i = 0; i < tcount2; i++)
	{
		for (int j=0; j<3; j++)
		{
			tarray[tcount1 + i].vertices[j] += vcount1;
		}
	}
	
	ret->set_data(varray, vcount, tarray, tcount);
	
	// cleanup
	delete [] varray;
	delete [] tarray;
}

/* Nicer JoinTriMesh - (JT)
 * This is a much better way to do things.
 */
TriMesh *join_tri_mesh(const TriMesh *m1, const TriMesh *m2) {
	TriMesh *mesh = new TriMesh;
	join_tri_mesh(mesh, m1, m2);
	return mesh;
}


