import { Module } from '@nestjs/common';
import { DBModule } from 'src/db/db.module';
import { UsersController } from './users.controller';
import { UsersService } from './users.service';

@Module({
	imports: [DBModule],
	controllers: [UsersController],
	providers: [UsersService],
	exports: [UsersService]
})
export class UsersModule {}

