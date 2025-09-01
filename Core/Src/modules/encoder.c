#include "headers/encoder.h"
#include < stdlib.h>
#include < stdio.h>
#include < math.h>

// aggiunto per powf float SAMPLING_PERIOD;
// seconds uint32_t HCLK;
// Vandermonde matrix in descending order of powers

static void build_vandermonde_desc(float32_t *t, float32_t *A)
{
	for (uint32_t i = 0; i < N_SAMPLES; i++)
	{
		for (int j = POLY_ORDER; j >= 0; j--)
		{
			A[i * (POLY_ORDER + 1) + (POLY_ORDER - j)] = powf(t[i], j);
		}
	}
}
static void compute_polynomial(encoder_t *encoder)
{
	/* Computes the polynomial coefficients for the position vector. This is used to smooth the position data over time. n > m (N_SAMPLES > N_COEFF) */
	float32_t t_normalized[N_SAMPLES];
	float32_t A_d[N_SAMPLES * N_COEFF];
	float32_t AT_d[N_COEFF * N_SAMPLES];
	float32_t ATA_d[N_COEFF * N_COEFF];
	float32_t ATA_inv_d[N_COEFF * N_COEFF];
	float32_t ATA_inv_AT_d[N_COEFF * N_SAMPLES];
	float32_t B_d[N_SAMPLES];
	float32_t P_d[N_COEFF]; // usa l'elemento più vecchio nel buffer come riferimento 
	uint8_t start_idx = encoder->vec_index; // 
	encoder->t_ref = encoder->timestamps[start_idx]; // Reference time for normalization 
	// riempi T e B per TUTTI i N_SAMPLES campioni (coerente con A: N_SAMPLES x N_COEFF) 
	for (uint8_t i = 0; i < N_SAMPLES; i++) { 
		uint8_t idx = (start_idx + i) % N_SAMPLES; 
		t_normalized[i] = (encoder->timestamps[idx] - encoder->t_ref)/1000.0f; // s 
		B_d[i] = encoder->positions[idx]; 
	} 
	arm_matrix_instance_f32 A, AT, ATA, ATA_inv, ATA_inv_AT, P, B; 
	build_vandermonde_desc(t_normalized, A_d); 
	arm_mat_init_f32(&A, N_SAMPLES, N_COEFF, A_d); 
	arm_mat_init_f32(&ATA, N_COEFF, N_COEFF, ATA_d); 
	arm_mat_init_f32(&ATA_inv, N_COEFF, N_COEFF, ATA_inv_d); 
	arm_mat_init_f32(&ATA_inv_AT, N_COEFF, N_SAMPLES, ATA_inv_AT_d); 
	arm_mat_init_f32(&P, N_COEFF, 1, P_d); 
	arm_mat_init_f32(&AT, N_COEFF, N_SAMPLES, AT_d); 
	arm_mat_init_f32(&B, N_SAMPLES, 1, B_d); 
	arm_mat_trans_f32(&A, &AT); // AT = A^T 
	arm_mat_mult_f32(&AT, &A, &ATA); // ATA = AT * A 
	arm_status status = arm_mat_inverse_f32(&ATA, &ATA_inv); 
	if(status == ARM_MATH_SUCCESS){ 
		arm_mat_mult_f32(&ATA_inv, &AT, &ATA_inv_AT); // ATA_inv_AT = ATA^-1 * AT 
		arm_mat_mult_f32(&ATA_inv_AT, &B, &P); // P = ATA_inv_AT * B 
		for(uint8_t i=0;i<N_COEFF;i++){ 
			encoder->polynomial[i] = P_d[i]; 
		} 
	} else { 
		for(uint8_t i=0;i<N_COEFF;i++){ 
			encoder->polynomial[i] = 0.0f; // Reset to zero on error 
		} 
		encoder->t_ref = HAL_GetTick(); // Reset reference time on error 
	} 
} 

static void compute_displacement(encoder_t *encoder){ 
	// encoder->old_displacement = encoder->displacement; // Save old displacement for speed calculation 
	encoder->displacement = 0.0f; 
	float current_time = (HAL_GetTick() - encoder->t_ref)/1000.0f; // Time since reference 
	for(int i = 0; i < N_COEFF; i++){ 
		encoder->displacement += encoder->polynomial[i] * powf(current_time, POLY_ORDER - i); 
	} 
} 

static void compute_speed(encoder_t *encoder){ 
	encoder->speed = (encoder->position*RCF - encoder->old_displacement) / SAMPLING_PERIOD; // Speed in radians per second 
	encoder->old_displacement = encoder->position * RCF; // 
	// encoder->speed = 0.0f; // 
	// float current_time = (HAL_GetTick() - encoder->t_ref)/1000.0f; // Time since reference 
	// for(uint8_t i = 0; i < POLY_ORDER; i++){ 
	// 	encoder->speed += (POLY_ORDER - i) *encoder->polynomial[i] * powf(current_time, POLY_ORDER - i - 1); 
	// } 
} 

void Encoder_init(encoder_t *encoder, TIM_HandleTypeDef *em_tim, TIM_HandleTypeDef *s_tim, int8_t direction_invert){ 
	/* Parameters: 
	 - encoder: Pointer to the encoder structure 
	 - em_tim: Pointer to the encoder mode timer handle 
	 - s_tim: Pointer to the sampling timer handle 
	 - direction_invert: Direction inversion flag 
	*/ 
	encoder->tim = em_tim->Instance; 
	encoder->tim->CCR3 = 1; 
	encoder->direction_invert = (direction_invert == 0) ? 1 : (direction_invert > 0 ? 1 : -1); 
	encoder->speed = 0; 
	uint32_t current_time = HAL_GetTick(); 
	for (uint8_t i = 0; i < N_SAMPLES; i++) { 
		encoder->timestamps[i] = current_time + i; 
		encoder->positions[i] = 0; 
	} 
	for (uint8_t i = 0; i < N_COEFF; i++) { 
		encoder->polynomial[i] = 0.0f; 
	} 
	encoder->position = 0; 
	encoder->vec_index = 0; 
	encoder->old_displacement=0.0f; 
	HCLK = HAL_RCC_GetHCLKFreq(); 
	SAMPLING_PERIOD =(float)(1+s_tim->Instance->ARR)*(1+s_tim->Instance->PSC)/HCLK; 
} 

void Encoder_read(encoder_t *encoder){ 
	compute_polynomial(encoder); 
	compute_displacement(encoder); 
	compute_speed(encoder); 
} 

void Encoder_event(encoder_t *encoder){ 
	encoder->direction = (encoder->tim->CR1 & TIM_CR1_DIR_Msk) >> TIM_CR1_DIR_Pos; 
	encoder->position += (encoder->direction ? -1 : 1) * encoder->direction_invert; 
	uint32_t current_time = HAL_GetTick(); // Store current time 
	// calcolo robusto dell'indice precedente nel buffer circolare 
	uint8_t prev = (encoder->vec_index + N_SAMPLES - 1) % N_SAMPLES; 
	if(encoder->timestamps[prev] == current_time){ 
		encoder->positions[prev] = encoder->position * RCF; // Convert to radians 
	} else { 
		encoder->timestamps[encoder->vec_index] = current_time; 
		encoder->positions[encoder->vec_index] = encoder->position * RCF; // Convert to radians 
		encoder->vec_index = (encoder->vec_index + 1) % N_SAMPLES; 
	} 
}