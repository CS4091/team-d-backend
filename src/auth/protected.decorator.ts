import { applyDecorators } from '@nestjs/common';
import { Reflector } from '@nestjs/core';
import { ApiHeader } from '@nestjs/swagger';

export const _Protected = Reflector.createDecorator();
export const Protected = () => applyDecorators(_Protected, ApiHeader({ name: 'Authorization', required: true }));

