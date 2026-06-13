import { useEffect, useState } from 'react';
import { Alert, Button, Col, Form, Row } from 'react-bootstrap';
import { NavLink } from 'react-router-dom';
import { useTranslation } from 'react-i18next';

import Section from '../Components/Section';
import WebApi from '../Services/WebApi';

type GameEntry = {
	gameId: number;
	enabled: boolean;
	order: number;
	name: string;
};

type MiniGameOptions = {
	enabled: boolean;
	defaultGameId: number;
	rhythmBpm: number;
	rhythmDifficulty: number;
	games: GameEntry[];
};

const DEFAULT_OPTIONS: MiniGameOptions = {
	enabled: true,
	defaultGameId: 1,
	rhythmBpm: 120,
	rhythmDifficulty: 2,
	games: [{ gameId: 1, enabled: true, order: 0, name: 'Rhythm Rush' }],
};

export default function MiniGamesPage() {
	const { t } = useTranslation('');
	const [options, setOptions] = useState<MiniGameOptions>(DEFAULT_OPTIONS);
	const [loading, setLoading] = useState(true);
	const [message, setMessage] = useState('');
	const [error, setError] = useState('');

	useEffect(() => {
		WebApi.getMiniGameOptions()
			.then(({ data }: { data: MiniGameOptions }) => setOptions(data))
			.catch(() => setError(t('MiniGames:error-message')))
			.finally(() => setLoading(false));
	}, []);

	const updateGame = (gameId: number, enabled: boolean) => {
		setOptions((current) => ({
			...current,
			games: current.games.map((game) =>
				game.gameId === gameId ? { ...game, enabled } : game,
			),
		}));
	};

	const save = async () => {
		setMessage('');
		setError('');
		try {
			const { data } = await WebApi.setMiniGameOptions(options);
			setOptions(data);
			setMessage(t('MiniGames:saved-message'));
		} catch {
			setError(t('MiniGames:error-message'));
		}
	};

	if (loading) {
		return <div className="spinner-border" role="status" />;
	}

	const enabledGames = options.games.filter((game) => game.enabled);

	return (
		<>
			<Alert variant="info">
				{t('MiniGames:description')}{' '}
				<NavLink to="/boot-mode-mapping">
					{t('Navigation:boot-mode-mapping-label')}
				</NavLink>
			</Alert>
			<Section title={t('MiniGames:title')}>
				<Form.Check
					type="switch"
					id="mini-games-enabled"
					label={t('MiniGames:enabled-label')}
					checked={options.enabled}
					onChange={(event) =>
						setOptions({ ...options, enabled: event.target.checked })
					}
				/>
			</Section>

			<Section title={t('MiniGames:games-label')}>
				{options.games.map((game) => (
					<Form.Check
						key={game.gameId}
						id={`mini-game-${game.gameId}`}
						label={game.name}
						checked={game.enabled}
						onChange={(event) => updateGame(game.gameId, event.target.checked)}
					/>
				))}
				<Form.Group as={Row} className="mt-3 align-items-center">
					<Form.Label column sm={4}>
						{t('MiniGames:default-game-label')}
					</Form.Label>
					<Col sm={5}>
						<Form.Select
							value={options.defaultGameId}
							disabled={enabledGames.length === 0}
							onChange={(event) =>
								setOptions({
									...options,
									defaultGameId: Number(event.target.value),
								})
							}
						>
							{enabledGames.map((game) => (
								<option key={game.gameId} value={game.gameId}>
									{game.name}
								</option>
							))}
						</Form.Select>
					</Col>
				</Form.Group>
			</Section>

			<Section title={t('MiniGames:rhythm-settings-label')}>
				<Form.Group as={Row} className="mb-3 align-items-center">
					<Form.Label column sm={4}>
						{t('MiniGames:bpm-label')}
					</Form.Label>
					<Col sm={5}>
						<Form.Control
							type="number"
							min={60}
							max={240}
							value={options.rhythmBpm}
							onChange={(event) =>
								setOptions({
									...options,
									rhythmBpm: Math.max(
										60,
										Math.min(240, Number(event.target.value)),
									),
								})
							}
						/>
					</Col>
				</Form.Group>
				<Form.Group as={Row} className="align-items-center">
					<Form.Label column sm={4}>
						{t('MiniGames:difficulty-label')}
					</Form.Label>
					<Col sm={5}>
						<Form.Select
							value={options.rhythmDifficulty}
							onChange={(event) =>
								setOptions({
									...options,
									rhythmDifficulty: Number(event.target.value),
								})
							}
						>
							<option value={1}>{t('MiniGames:difficulty-easy')}</option>
							<option value={2}>{t('MiniGames:difficulty-normal')}</option>
							<option value={3}>{t('MiniGames:difficulty-hard')}</option>
						</Form.Select>
					</Col>
				</Form.Group>
			</Section>

			<div className="d-flex align-items-center gap-3 mt-3">
				<Button onClick={save}>{t('Common:button-save-label')}</Button>
				{message && <span className="text-success">{message}</span>}
				{error && <span className="text-danger">{error}</span>}
			</div>
		</>
	);
}
