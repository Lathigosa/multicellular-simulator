// Bla

#define TEMPLATE_APPEND_KERNEL(TYPE, BYTE_SIZE) \
kernel void append_particles_##BYTE_SIZE (	global TYPE * out_buffer, \
										global uint* in_new_particles, \
										int append_index) \
{ \
	out_buffer[get_global_id(0) + append_index] = out_buffer[in_new_particles[get_global_id(0)]]; \
}

// TODO: perhaps double buffer to prevent race conditions! Is it necessary?

// Define a bunch of different deletion kernels for different byte sizes:
TEMPLATE_APPEND_KERNEL(uchar, 1);
TEMPLATE_APPEND_KERNEL(ushort, 2);
TEMPLATE_APPEND_KERNEL(uint, 4);
TEMPLATE_APPEND_KERNEL(ulong, 8);

TEMPLATE_APPEND_KERNEL(uint4, 16);
TEMPLATE_APPEND_KERNEL(uint8, 32);
TEMPLATE_APPEND_KERNEL(uint16, 64);
TEMPLATE_APPEND_KERNEL(ulong16, 128);

kernel void append_buffer(	global float4* out_buffer,
							global uint* in_new_cells,
							int append_index)
{
// TODO: perhaps double buffer to prevent race conditions! Is it necessary?
	out_buffer[get_global_id(0) + append_index] = out_buffer[in_new_cells[get_global_id(0)]];
};

// Array types:
// Call with global_id(0) \in [0, array_size], global_id(1) \in [0, amount_of_particles_to_copy]

#define TEMPLATE_APPEND_ARRAY_KERNEL(TYPE, BYTE_SIZE) \
kernel void append_particles_array_##BYTE_SIZE (	global TYPE * out_buffer, \
										global uint* in_new_particles, \
										int append_index, \
										int array_size) \
{ \
	uint current_particle = in_new_particles[get_global_id(1)]; \
	uint new_index = (current_particle + append_index)*array_size + get_global_id(0); \
	uint old_index = (current_particle)*array_size + get_global_id(0); \
	out_buffer[index] = out_buffer[old_index]; \
}

// Define a bunch of different deletion kernels for different byte sizes:
TEMPLATE_APPEND_ARRAY_KERNEL(uchar, 1);
TEMPLATE_APPEND_ARRAY_KERNEL(ushort, 2);
TEMPLATE_APPEND_ARRAY_KERNEL(uint, 4);
TEMPLATE_APPEND_ARRAY_KERNEL(ulong, 8);

TEMPLATE_APPEND_ARRAY_KERNEL(uint4, 16);
TEMPLATE_APPEND_ARRAY_KERNEL(uint8, 32);
TEMPLATE_APPEND_ARRAY_KERNEL(uint16, 64);
TEMPLATE_APPEND_ARRAY_KERNEL(ulong16, 128);
